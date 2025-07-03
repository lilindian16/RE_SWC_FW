/**
 * Rotary Encoder Steering Wheel Controller Firmware
 *
 * Control car stereos with KEY input using a rotary encoder
 *
 * MCU: ATtiny414
 * F_CPU: 1kHz
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
 * TODO:
 * Double press encoder switch to activate programming mode
 * Hold the SWC output longer
 * Improve input to SWC output UX
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
/* We must over-write F_CPU with our required F_CPU before including delay src */
#ifdef F_CPU
#undef F_CPU
#define F_CPU 1000L
#endif
#include <util/delay.h>
#include <stdbool.h>

/* MACROS */
const int SWC_OUTPUT_ENABLE_SHORT_HOLD_MS = 80;  // Time required for radio to read signal
const int SWC_OUTPUT_ENABLE_LONG_HOLD_MS = 4000; // Time required for radio to read claibration signal
const int SWC_OUTPUT_DISABLE_MS = 20;            // Time required until we send another signal

#define BUTTON_TIMER_TICKS_COUNT 250

/* Globals */
volatile int8_t encoder_count = 0;
const int8_t min_encoder_count = -5;
const int8_t max_encoder_count = 5;
volatile bool encoder_count_updated = false;
volatile bool encoder_button_pressed = false;
volatile bool encoder_button_double_pressed = false;
volatile bool button_timer_started = false;

ISR(TCB0_INT_vect)
{
  /* If we reach here, the button was pressed and the time required was elapsed without it
  being pressed again */
  /* Clear the ISR */
  TCB0_INTFLAGS = TCB_CAPT_bm;
  /* Clear the flags */
  button_timer_started = false;
  /* Set the button pressed flag */
  encoder_button_pressed = true;
}

inline void start_button_timer(void)
{
  /* Make sure the clock is disabled before configuring it */
  TCB0_CTRLA = 0x00;
  /* Set the timer mode */
  TCB0_CTRLB = (TCB_CNTMODE_SINGLE_gc);
  /* Set the count to 0 */
  TCB0_CNT = 0;
  /* Set the CCMP value (TOP) */
  /* We want the timer to trigger event after 500ms. @500hz, 500ms would result in 250 ticks */
  TCB0_CCMP = BUTTON_TIMER_TICKS_COUNT;
  /* Enable the interrupt */
  TCB0_INTCTRL = TCB_CAPT_bm;
  /* Set the clock freq -> div 2 = 500hz & enable timer to run */
  TCB0_CTRLA = (TCB_ENABLE_bm | TCB_CLKSEL_CLKDIV2_gc);
}

ISR(PORTA_PORT_vect)
{
  /* Cear the interrupt flag. The only input to trigger ISR is the encoder switch */
  PORTA.INTFLAGS = PIN2_bm;

  /** LOGIC:
   * -- If timer has not been started, start timer (250ms)
   * -- Elif timer has been started & is not expired, we have a double press. Reset and end timer
   *
   * If timer expired, we have a single press
   */

  if (!button_timer_started)
  {
    button_timer_started = true;
    start_button_timer();
    return;
  }
  /* If we have reached here, we have pressed the button again before the timer finished */
  /* Disable the timer and set the flag */
  TCB0_CTRLA = 0x00;
  button_timer_started = false;
  encoder_button_double_pressed = true;
}

ISR(PORTB_PORT_vect)
{
  /* Encoder A triggered interrupt */
  PORTB.INTFLAGS = PIN2_bm;        // Clear the interrupt flag
  uint8_t port_a_state = PORTA.IN; // We now need to check encoder B state on Port A
  if (port_a_state & PIN6_bm)
  {
    /* We have a CW rotation */
    if (encoder_count < max_encoder_count)
    {
      encoder_count++;
      return;
    }
  }
  else
  {
    /* We have a CCW rotation */
    if (encoder_count > min_encoder_count)
    {
      encoder_count--;
    }
  }
}

int main(void)
{
  /* Disable CLKOUT. Set clock src to internal 32kHz */
  _PROTECTED_WRITE(CLKCTRL_MCLKCTRLA, CLKCTRL_CLKSEL_OSCULP32K_gc);

  /* Enable prescaler. Set divider to 32x -> FCPU = 1kHz */
  _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, (CLKCTRL_PEN_bm | CLKCTRL_PDIV_32X_gc));

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
  PORTB.PIN2CTRL |= PORT_ISC_RISING_gc;

  /* Set encoder B input PU no ISR */
  PORTA.PIN6CTRL = PORT_PULLUPEN_bm; // Encoder B in

  /* Set RXD input PU to avoid floating */
  PORTB.PIN3CTRL = PORT_PULLUPEN_bm;

  /* Pulse the activity LED so we know we are up and running */
  for (uint8_t i = 0; i < 2; i++)
  {
    PORTA.OUTSET = PIN4_bm; // Turn on LED
    _delay_ms(100);
    PORTA.OUTCLR = PIN4_bm; // Turn off LED
    _delay_ms(100);
  }

  sei(); // Global interrupt enable

  while (1)
  {
    while (encoder_count || button_timer_started || encoder_button_pressed || encoder_button_double_pressed)
    {
      if (encoder_count)
      {
        PORTA.OUTSET = PIN4_bm; // Turn on LED
        if (encoder_count > 0)
        {
          /* SWC OUT 1 to increase the volume */
          encoder_count--;
          PORTA.OUTSET = PIN7_bm;
          if (encoder_button_double_pressed)
          {
            encoder_button_double_pressed = false;
            _delay_ms(SWC_OUTPUT_ENABLE_LONG_HOLD_MS);
            encoder_count = 0;
          }
          else
          {
            _delay_ms(SWC_OUTPUT_ENABLE_SHORT_HOLD_MS);
          }
          PORTA.OUTCLR = PIN7_bm;
          _delay_ms(SWC_OUTPUT_DISABLE_MS);
        }

        else
        {
          /* SWC OUT 2 to decrease the volume */
          encoder_count++;
          PORTB.OUTSET = PIN0_bm;
          if (encoder_button_double_pressed)
          {
            encoder_button_double_pressed = false;
            _delay_ms(SWC_OUTPUT_ENABLE_LONG_HOLD_MS);
            encoder_count = 0;
          }
          else
          {
            _delay_ms(SWC_OUTPUT_ENABLE_SHORT_HOLD_MS);
          }

          PORTB.OUTCLR = PIN0_bm;
          _delay_ms(SWC_OUTPUT_DISABLE_MS);
        }
        PORTA.OUTCLR = PIN4_bm; // Turn off LED
      }
      if (encoder_button_pressed)
      {
        encoder_button_pressed = false;
        PORTA.OUTSET = PIN4_bm; // Turn on LED
        PORTB.OUTSET = PIN1_bm;
        if (encoder_button_double_pressed)
        {
          encoder_button_double_pressed = false;
          _delay_ms(SWC_OUTPUT_ENABLE_LONG_HOLD_MS);
        }
        else
        {
          _delay_ms(SWC_OUTPUT_ENABLE_SHORT_HOLD_MS);
        }
        PORTB.OUTCLR = PIN1_bm;
        _delay_ms(SWC_OUTPUT_DISABLE_MS);
        PORTA.OUTCLR = PIN4_bm; // Turn off LED
      }

      if (encoder_button_double_pressed)
      {
        for (uint8_t i = 0; i < 2; i++)
        {
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
    SLPCTRL.CTRLA = 0x00;                                        // We return here when device wakes up. Disable sleep
  }
  return 0;
}