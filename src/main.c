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

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
/* We must over-write F_CPU with our required F_CPU before including delay src */
#ifdef F_CPU
#undef F_CPU
#define F_CPU 32000L
#endif
#include <util/delay.h>

/* Include the headunit configs */
#include "headunit_configs.h"

/* Amount of ticks required for the button press timer */
#define BUTTON_TIMER_TICKS_COUNT 8000

/* Ceiling and floor for encoder rotation counts */
#define MIN_ENCODER_COUNT -2
#define MAX_ENCODER_COUNT 2

/* Encoder state flags */
#define ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM 0x01
#define ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM 0x02
#define ENCODER_FLAG_BUTTON_TIMER_STARTED_BM        0x04

#if !defined(SWC_OUTPUT_DISABLE_MS) || !defined(SWC_OUTPUT_ENABLE_SHORT_HOLD_MS)                                       \
    || !defined(SWC_OUTPUT_ENABLE_LONG_HOLD_MS)
#warning "Not all SWC parameters are defined. Check the headunit_configs.h file"
#endif

/* Signed byte to store encoder count. Will be updated in ISR so must be volatile */
volatile int8_t encoder_count = 0;
/* Byte to store flags for encoder events. Updated in ISR so must be volatile */
volatile uint8_t encoder_flags = 0;

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
    /* We want the timer to trigger event after 500ms. @16kHz, 500ms would result in 8,000 ticks */
    TCB0_CCMP = BUTTON_TIMER_TICKS_COUNT;
    /* Enable the interrupt */
    TCB0_INTCTRL = TCB_CAPT_bm;
    /* Set the clock freq -> div 2 = 500hz & enable timer to run */
    TCB0_CTRLA |= TCB_ENABLE_bm;
}

ISR(TCB0_INT_vect) {
    /* If this ISR is called, the button was pressed and the timer completed without it
    being pressed again */
    /* Clear the ISR */
    TCB0_INTFLAGS = TCB_CAPT_bm;
    /* Clear the flags */
    encoder_flags &= ~(ENCODER_FLAG_BUTTON_TIMER_STARTED_BM);
    /* The button has been pressed once. Set the flag */
    encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM;
}

ISR(PORTA_PORT_vect) {
    /* Cear the interrupt flag. The only input to trigger ISR is the encoder switch */
    PORTA.INTFLAGS = PIN2_bm;

    if (!(encoder_flags & ENCODER_FLAG_BUTTON_TIMER_STARTED_BM)) {
        encoder_flags |= ENCODER_FLAG_BUTTON_TIMER_STARTED_BM; // Set the timer started flag
        start_button_timer();
        return;
    }
    /* If we have reached here, we have pressed the button again before the timer finished */
    /* Disable the timer and set the flag */
    TCB0_CTRLA = 0x00;
    encoder_flags &= ~(ENCODER_FLAG_BUTTON_TIMER_STARTED_BM);     // Clear the timer started flag
    encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM; // Set the double pressed flag
    /* Also clear the encoder count so we don't false trigger another SWC output */
    encoder_count = 0;
}

ISR(PORTB_PORT_vect) {
    /* Encoder A triggered interrupt */
    PORTB.INTFLAGS = PIN2_bm; // Clear the interrupt flag
    if (PORTA_IN & PIN6_bm)   // We now need to check encoder B state on Port A
    {
        /* We have a CCW rotation */
        if (encoder_count < MAX_ENCODER_COUNT) {
            encoder_count += 1;
        }
        return;
    }
    /* We have a CW rotation */
    if (encoder_count > MIN_ENCODER_COUNT) {
        encoder_count -= 1;
    }
}

int main(void) {
    /* Disable CLKOUT. Set clock src to internal 32kHz */
    _PROTECTED_WRITE(CLKCTRL_MCLKCTRLA, CLKCTRL_CLKSEL_OSCULP32K_gc);

    /* Disable prescaler. F_CPU = 32kHZ */
    _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, 0x00);

    /* Set output direction for Port A. All others will be input */
    PORTA.DIR = (PIN1_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN7_bm);
    /* Set output direction for Port B. All others will be input */
    PORTB.DIR = (PIN0_bm | PIN1_bm);

    /* Set encoder switch input PU with falling edge ISR */
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc; // Encoder SW 1

    /* Set encoder A input PU */
    PORTB.PIN2CTRL = PORT_PULLUPEN_bm; // Encoder A in
    /* Allow the filter cap to charge before enabling ISR to avoid early trigger */
    _delay_ms(100);
    /* Enable ISR for encoder A */
    PORTB.PIN2CTRL |= PORT_ISC_FALLING_gc;

    /* Set encoder B input PU no ISR */
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm; // Encoder B in

    /* Set RXD input PU to avoid floating */
    PORTB.PIN3CTRL = PORT_PULLUPEN_bm;

    /* Pulse the activity LED so we know we are up and running */
    for (uint8_t i = 0; i < 2; i++) {
        PORTA.OUTSET = PIN4_bm; // Turn on LED
        _delay_ms(100);
        PORTA.OUTCLR = PIN4_bm; // Turn off LED
        _delay_ms(100);
    }

    sei(); // Global interrupt enable

    while (1) {
        while (encoder_count || encoder_flags) {
            if (encoder_count != 0) {
                PORTA.OUTSET = PIN4_bm; // Turn on LED
                if (encoder_count > 0) {
                    /* SWC OUT 1 to increase the volume */
                    encoder_count--;
                    PORTA.OUTSET = PIN7_bm;
                    if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM) {
                        encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM); // Clear the flag
                        _delay_ms(SWC_OUTPUT_ENABLE_LONG_HOLD_MS);
                        encoder_count = 0;
                    } else {
                        _delay_ms(SWC_OUTPUT_ENABLE_SHORT_HOLD_MS);
                    }
                    PORTA.OUTCLR = PIN7_bm;
                    _delay_ms(SWC_OUTPUT_DISABLE_MS);
                }

                else {
                    /* SWC OUT 2 to decrease the volume */
                    encoder_count++;
                    PORTB.OUTSET = PIN0_bm;
                    if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM) {
                        encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM); // Clear the flag
                        _delay_ms(SWC_OUTPUT_ENABLE_LONG_HOLD_MS);
                        encoder_count = 0;
                    } else {
                        _delay_ms(SWC_OUTPUT_ENABLE_SHORT_HOLD_MS);
                    }
                    PORTB.OUTCLR = PIN0_bm;
                    _delay_ms(SWC_OUTPUT_DISABLE_MS);
                }
                PORTA.OUTCLR = PIN4_bm; // Turn off LED
            }
            if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM) {
                encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM); // Clear the flag
                PORTA.OUTSET = PIN4_bm;                                          // Turn on LED
                PORTB.OUTSET = PIN1_bm;
                if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM) {
                    encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM);
                    _delay_ms(SWC_OUTPUT_ENABLE_LONG_HOLD_MS);
                } else {
                    _delay_ms(SWC_OUTPUT_ENABLE_SHORT_HOLD_MS);
                }
                PORTB.OUTCLR = PIN1_bm;
                _delay_ms(SWC_OUTPUT_DISABLE_MS);
                PORTA.OUTCLR = PIN4_bm; // Turn off LED
            }

            if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM) {
                for (uint8_t i = 0; i < 2; i++) {
                    PORTA.OUTSET = PIN4_bm;
                    _delay_ms(50);
                    PORTA_OUTCLR = PIN4_bm;
                    _delay_ms(50);
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