#include "generic_resistive_headunit.h"

#include <avr/io.h>
#include <util/delay.h>

#define SWC_OUTPUT_ENABLE_SHORT_HOLD_mS 80
#define SWC_OUTPUT_ENABLE_LONG_HOLD_mS  4000
#define SWC_OUTPUT_DISABLE_mS           20

void generic_resistive_headunit_volume_up(bool hold_output) {
    PORTA.OUTSET = PIN7_bm;
    if (hold_output) {
        _delay_ms(SWC_OUTPUT_ENABLE_LONG_HOLD_mS);
    } else {
        _delay_ms(SWC_OUTPUT_ENABLE_SHORT_HOLD_mS);
    }
    PORTA.OUTCLR = PIN7_bm;
    _delay_ms(SWC_OUTPUT_DISABLE_mS);
}

void generic_resistive_headunit_volume_down(bool hold_output) {
    PORTB.OUTSET = PIN0_bm;
    if (hold_output) {
        _delay_ms(SWC_OUTPUT_ENABLE_LONG_HOLD_mS);
    } else {
        _delay_ms(SWC_OUTPUT_ENABLE_SHORT_HOLD_mS);
    }
    PORTB.OUTCLR = PIN0_bm;
    _delay_ms(SWC_OUTPUT_DISABLE_mS);
}

void generic_resistive_headunit_button_short_press(bool hold_output) {
    PORTB.OUTSET = PIN1_bm;
    if (hold_output) {
        _delay_ms(SWC_OUTPUT_ENABLE_LONG_HOLD_mS);
    } else {
        _delay_ms(SWC_OUTPUT_ENABLE_SHORT_HOLD_mS);
    }
    PORTB.OUTCLR = PIN1_bm; // Disable SWC output
    _delay_ms(SWC_OUTPUT_DISABLE_mS);
}

void generic_resistive_headunit_button_held(bool hold_output) {
    PORTB.OUTSET = PIN1_bm; // Enable SWC output
    while (!(PORTA_IN & PIN2_bm)) {
        _delay_ms(10);
    }
    PORTB.OUTCLR = PIN1_bm; // Disable SWC output
    _delay_ms(SWC_OUTPUT_DISABLE_mS);
}

void generic_resistive_headunit_button_double_press(bool hold_output) {
    /* For now, all we do is flash the LED while we are in learning mode */
    for (uint8_t i = 0; i < 2; i++) {
        PORTA.OUTSET = PIN4_bm;
        _delay_ms(50);
        PORTA_OUTCLR = PIN4_bm;
        _delay_ms(50);
    }
}