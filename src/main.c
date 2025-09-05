/**
 * Rotary Encoder Steering Wheel Controller Firmware
 *
 * Control car stereos with KEY input using a rotary encoder
 *
 * MCU: ATtiny414
 * F_CPU: 20MHz but we will use the 32kHz low power internal clock
 *
 * Pinout:
 * PA1 -> TXD (OUT)
 * PA2 -> ENCODER_SW1_IN (IN PU)
 * PA3 -> EXTRA_IO (OUT)
 * PA4 -> LED_STAT (OUT)
 * PA5 -> EXTRA_IO (OUT)
 * PA6 -> ENCODER_B_IN (IN PU)
 * PA7 -> SWC_OUT_1 (OUT)
 *
 * PB0 -> SWC_OUT_2 (OUT)
 * PB1 -> SWC_OUT_3 (OUT)
 * PB2 -> ENC_A_IN (IN PU)
 * PB3 -> RXD (IN PU)
 *
 * Note: Any floating input pins must be pulled-up / down to avoid extra power draw in sleep mode
 *
 */

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

/* Include the headunit configs */
#include <headunit_swc.h>

/* Amount of ticks required for the button press timer */
#define BUTTON_TIMER_PERIOD_mS 500
#define BUTTON_TIMER_PRESCALER 2
#define TIMER_TICKS_PER_mS     (F_CPU / BUTTON_TIMER_PRESCALER / 1000)

/* Ceiling and floor for encoder rotation counts */
#define MIN_ENCODER_COUNT -1
#define MAX_ENCODER_COUNT 1

/* Encoder state flags */
#define ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM (1 << 0)
#define ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM (1 << 1)
#define ENCODER_FLAG_ENCODER_BUTTON_HELD_BM         (1 << 2)
#define ENCODER_FLAG_BUTTON_TIMER_STARTED_BM        (1 << 3)

#define EEPROM_START_ADDRESS          0x00
#define EEPROM_HEADUNIT_BRAND_ADDRESS 0x04

/**
 *  We need to define our own eeprom_is_ready and eeprom_busy_wait functions since the toolchain defined these incorrect
 * :(
 */
#ifdef eeprom_is_ready
#undef eeprom_is_ready
#define eeprom_is_ready() bit_is_clear(NVMCTRL_STATUS, NVMCTRL_EEBUSY_bm)
#endif
#ifdef eeprom_busy_wait
#undef eeprom_busy_wait
#define eeprom_busy_wait()                                                                                             \
    do {                                                                                                               \
    } while (!eeprom_is_ready())
#endif

typedef enum { PROGRAM_STATE_CHANGING_HEADUNIT_BRAND, PRORGAM_STATE_NORMAL } Program_State_t;

/* Create the program state machine and default to normal */
Program_State_t current_program_state = PRORGAM_STATE_NORMAL;

/* Set the HU brand as GENERIC to begin with. It may be overwritten by EEPROM settings */
static Headunit_Brand_t current_headunit_brand = HEADUNIT_GENERIC_RESISTIVE;

/* Signed byte to store encoder count. Will be updated in ISR so must be volatile */
volatile int8_t encoder_count = 0;

/* Byte to store flags for encoder events. Updated in ISR so must be volatile */
volatile uint8_t encoder_flags = 0;

/* Create a tick int so we can increment the amount of ms a timer has been running for */
volatile int timer_ms_ticks = 0;

static inline void start_button_timer(void) {
    /* Make sure the clock is disabled before configuring it */
    TCB0_CTRLA = 0x00;
    /* Set the timer mode */
    TCB0_CTRLB = (TCB_CNTMODE_SINGLE_gc);
    /* Set the count to 0 */
    TCB0_CNT = 0;
    /* Set the prescaler to div 2 */
    TCB0_CTRLA = TCB_CLKSEL_CLKDIV2_gc;
    /* Set the CCMP value (TOP) */
    TCB0_CCMP = TIMER_TICKS_PER_mS;
    /* Enable the interrupt */
    TCB0_INTCTRL = TCB_CAPT_bm;
    /* Set the clock freq -> div 2 & enable timer to run */
    TCB0_CTRLA |= TCB_ENABLE_bm;
}

ISR(TCB0_INT_vect) {
    /* Clear the ISR */
    TCB0_INTFLAGS = TCB_CAPT_bm;

    /* Get the current value of ticks - this is volatile so disable interrupts */
    cli();
    timer_ms_ticks++;
    int ticks = timer_ms_ticks;
    sei();

    if (ticks >= BUTTON_TIMER_PERIOD_mS) {
        /* Clear the flags */
        encoder_flags &= ~(ENCODER_FLAG_BUTTON_TIMER_STARTED_BM);

        /* Reset the timer ticks */
        cli();
        timer_ms_ticks = 0;
        sei();

        /* Check the state of the button input. If it is still high, we have a long press. If not, we have a
         * short press */
        if (PORTA_IN & PIN2_bm) {
            /* Button was pressed and is now released. Enable button pressed flag */
            encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM;
        }

        else {
            /* The button is still pressed. Enable the button held flag */
            encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_HELD_BM;
        }
        return;
    }

    /* If we reach here, we have not reached the timeout so restart the timer :) */
    TCB0_CNT = 0;
    TCB0_CTRLA |= TCB_ENABLE_bm;
}

ISR(PORTA_PORT_vect) {
    /* Cear the interrupt flag. The only input to trigger ISR is the encoder switch */
    PORTA.INTFLAGS = PIN2_bm;

    /* Check if the button press timer has been started */
    if (!(encoder_flags & ENCODER_FLAG_BUTTON_TIMER_STARTED_BM)) {
        /* Timer not started, this is the first button press. Set the timer started
        flag and start the timer */
        encoder_flags |= ENCODER_FLAG_BUTTON_TIMER_STARTED_BM;
        start_button_timer();
        return;
    }
    /* If we have reached here, we have pressed the button again before the timer finished
    Disable the timer and set the button double press flag */
    TCB0_CTRLA = 0x00;
    encoder_flags &= ~(ENCODER_FLAG_BUTTON_TIMER_STARTED_BM);     // Clear the timer started flag
    encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM; // Set the double pressed flag
    /* Also clear the encoder count so we don't false trigger another SWC output */
    encoder_count = 0;
}

ISR(PORTB_PORT_vect) {
    /* Encoder A triggered interrupt */
    PORTB.INTFLAGS = PIN2_bm; // Clear the interrupt flag
    /* We now need to check encoder B state on Port A */
    if (PORTA_IN & PIN6_bm) {
        /* We have a CCW rotation */
        if (encoder_count > MIN_ENCODER_COUNT) {
            encoder_count -= 1;
        }
        return;
    }
    /* We have a CW rotation */
    if (encoder_count < MAX_ENCODER_COUNT) {
        encoder_count += 1;
    }
}

int main(void) {
    /* Disable CLKOUT. Set clock src to internal 20MHz */
    _PROTECTED_WRITE(CLKCTRL_MCLKCTRLA, CLKCTRL_CLKSEL_OSC20M_gc);
    /* Enable prescaler with 10x div. F_CPU = 2MHz */
    _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, (0x01 | CLKCTRL_PDIV_10X_gc));

    /* Read the EEPROM to see if we have ever used it */
    eeprom_busy_wait();
    uint32_t eeprom_header = eeprom_read_dword((uint32_t*)EEPROM_START_ADDRESS);
    if (eeprom_header != 0xDEADBEEF) {
        eeprom_busy_wait();
        eeprom_update_dword((uint32_t*)EEPROM_START_ADDRESS, 0xDEADBEEF);
        eeprom_busy_wait();
        eeprom_update_byte((uint8_t*)EEPROM_HEADUNIT_BRAND_ADDRESS, (uint8_t)HEADUNIT_GENERIC_RESISTIVE); // Default to
                                                                                                          // resistive
                                                                                                          // HU
        current_headunit_brand = HEADUNIT_GENERIC_RESISTIVE;
    }

    else {
        eeprom_busy_wait();
        current_headunit_brand = (Headunit_Brand_t)eeprom_read_byte((uint8_t*)EEPROM_HEADUNIT_BRAND_ADDRESS);
    }

    /* Set output direction for Port A. All others will be input */
    PORTA.DIR = (PIN1_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN7_bm);
    /* Set output direction for Port B. All others will be input */
    PORTB.DIR = (PIN0_bm | PIN1_bm);
    /* Set encoder switch input PU with falling edge ISR */
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;
    /* Set encoder A input PU */
    PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
    /* Allow the filter cap to charge before enabling ISR to avoid early trigger */
    _delay_ms(100);
    /* Enable ISR for encoder A */
    PORTB.PIN2CTRL |= PORT_ISC_FALLING_gc;
    /* Set encoder B input PU no ISR */
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm; // Encoder B in
    /* Set RXD input PU to avoid floating */
    PORTB.PIN3CTRL = PORT_PULLUPEN_bm;

    uint8_t count = 0;
    while (!(PORTA_IN & PIN2_bm) && count < 5) {
        _delay_ms(1000);
        count++;
    }

    if (count == 5) {
        current_program_state = PROGRAM_STATE_CHANGING_HEADUNIT_BRAND;
        /* Pulse the LED while the button is still being held */
        while (!(PORTA_IN & PIN2_bm)) {
            PORTA.OUTSET = PIN4_bm; // Turn on LED
            _delay_ms(100);
            PORTA.OUTCLR = PIN4_bm; // Turn off LED
            _delay_ms(100);
        }

        /* Enable all interrupts so we can capture button inputs asynchronously */
        sei();

        /* Reset to the starting value */
        current_headunit_brand = HEADUNIT_BRAND_UNKNOWN;

        /* Wait until the user has held the button to confirm their input */
        while (!(encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_HELD_BM)) {
            /* Flash the LED each time the button is pressed to show the increment of the counter */
            if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM) {
                encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM); // Clear the flag
                /* Increment the current headunit index if we are within the bounds */
                if (current_headunit_brand != HEADUNIT_BRAND_ERROR) {
                    current_headunit_brand = (Headunit_Brand_t)((uint8_t)current_headunit_brand + 1);
                }
                PORTA.OUTSET = PIN4_bm; // Turn on LED
                _delay_ms(100);
                PORTA.OUTCLR = PIN4_bm; // Turn off LED
                _delay_ms(100);
            };
        }

        /* We reach here after holding the button to confirm our entry */
        encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_HELD_BM); // Clear the flag
        /* We keep the LED on while the user has the button held. Turn it off when released */
        while (!(PORTA_IN & PIN2_bm)) {
            PORTA.OUTSET = PIN4_bm;
        }
        PORTA.OUTCLR = PIN4_bm; // Turn off LED

        /* Write the headunit brand enum to EEPROM */
        eeprom_busy_wait();
        eeprom_update_byte((uint8_t*)EEPROM_HEADUNIT_BRAND_ADDRESS, (uint8_t)current_headunit_brand);
        _delay_ms(2500);
    }

    /* We didn't enter config mode so wait to show the LED */
    else {
        _delay_ms(500);
    }

    /* Default back to the base HU if there is an error when retrieving the HU brand saved */
    if (current_headunit_brand == HEADUNIT_BRAND_UNKNOWN || current_headunit_brand == HEADUNIT_BRAND_ERROR) {
        current_headunit_brand = HEADUNIT_GENERIC_RESISTIVE;
    }

    /* Pulse the LED x number of times to show index of current headunit selected */
    for (uint8_t i = 0; i < (uint8_t)current_headunit_brand; i++) {
        PORTA.OUTSET = PIN4_bm; // Turn on LED
        _delay_ms(100);
        PORTA.OUTCLR = PIN4_bm; // Turn off LED
        _delay_ms(100);
    }

    init_headunit_swc(&current_headunit_brand);

    /* Make sure all interrupts are enabled */
    sei();

    /* Reset all the encoder flags to avoid any errors */
    encoder_flags = 0;

    while (1) {
        /* Check if we have any input events */
        while (encoder_count || encoder_flags) {
            if (encoder_count != 0) {
                PORTA.OUTSET = PIN4_bm; // Turn on LED
                if (encoder_count > 0) {
                    /* SWC OUT 1 to increase the volume */
                    encoder_count--;
                    if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM) {
                        encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM); // Clear the flag
                        headunit_volume_up(true);
                    } else {
                        headunit_volume_up(false);
                    }
                }

                else {
                    /* SWC OUT 2 to decrease the volume */
                    encoder_count++;
                    if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM) {
                        encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM); // Clear the flag
                        headunit_volume_down(true);
                    } else {
                        headunit_volume_down(false);
                    }
                }
                PORTA.OUTCLR = PIN4_bm; // Turn off LED
            }
            if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM) {
                encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM); // Clear the flag
                PORTA.OUTSET = PIN4_bm;                                          // Turn on LED
                if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM) {
                    encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM);
                    headunit_button_short_press(true);
                } else {
                    headunit_button_short_press(false);
                }
                PORTA.OUTCLR = PIN4_bm; // Turn off LED
            }

            if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_HELD_BM) {
                encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_HELD_BM); // Clear the flag
                PORTA.OUTSET = PIN4_bm;                                  // Turn on LED
                headunit_button_held(false);
                PORTA.OUTCLR = PIN4_bm; // Turn off LED
            }

            if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM) {
                headunit_button_double_pressed(false);
                if (current_headunit_brand != HEADUNIT_GENERIC_RESISTIVE) {
                    encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM); // Clear the flag
                }
            }
        }

        /* Enter sleep mode when tasks completed */
        cli();                                                       // Disable interrupts
        SLPCTRL.CTRLA = (SLEEP_ENABLED_gc | SLPCTRL_SMODE_PDOWN_gc); // Enable sleep control. Set mode to POWERDOWN
        sei();                                                       // Enable global interrupts
        sleep_cpu();                                                 // Enter sleep mode
        SLPCTRL.CTRLA = 0x00; // We return here when device wakes up. Disable sleep
    }
    return 0;
}