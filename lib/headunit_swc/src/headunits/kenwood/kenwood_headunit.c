/**
 * Kenwood steering wheel control interface
 * Refer to img/ folder for logic analyser captures showing
 * how this protocol was reverse engineered
 */

#include "kenwood_headunit.h"

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define KENWOOD_DATA_LENGTH_BITS 8
/* Define the tick resolution in micro-seconds. This is the time required for a single bit of data .. hard to explain,
 * easier to show in the logic analyser captures */
#define KENWOOD_TICK_RESOLUTION_uS               530
#define KENWOOD_SHORT_PULSE                      KENWOOD_TICK_RESOLUTION_uS
#define KENWOOD_LONG_PULSE                       (KENWOOD_TICK_RESOLUTION_uS * 3)
#define KENWOOD_PREAMBLE_LONG_PULSE_DURATION_mS  9
#define KENWOOD_PREAMBLE_PULSE_PAUSE_DURATION_mS 4
#define KENWOOD_MESSAGE_DELAY_mS                 5

#define KENWOOD_ADDRESS          0xB9
#define KENWOOD_ADDRESS_INVERTED 0x46

enum Kenwood_SWC_Command {
    KENWOOD_PREVIOUS_TRACK = 0x0A,
    KENWOOD_NEXT_TRACK = 0x0B,
    KENWOOD_PLAY_PAUSE = 0x0E,
    KENWOOD_VOLUME_UP = 0x14,
    KENWOOD_VOLUME_DOWN = 0x15,
    KENWOOD_MUTE = 0x16,
};

static void kenwood_binary_one(void) {
    PORTA.OUTSET = PIN7_bm;
    _delay_us(KENWOOD_SHORT_PULSE);
    PORTA.OUTCLR = PIN7_bm;
    _delay_us(KENWOOD_LONG_PULSE);
}

static void kenwood_binary_zero(void) {
    PORTA.OUTSET = PIN7_bm;
    _delay_us(KENWOOD_SHORT_PULSE);
    PORTA.OUTCLR = PIN7_bm;
    _delay_us(KENWOOD_SHORT_PULSE);
}

static void kenwood_preamble(void) {
    PORTA.OUTSET = PIN7_bm;
    _delay_ms(KENWOOD_PREAMBLE_LONG_PULSE_DURATION_mS);
    PORTA.OUTCLR = PIN7_bm;
    _delay_ms(KENWOOD_PREAMBLE_PULSE_PAUSE_DURATION_mS);
}

static void kenwood_postamble(void) {
    PORTA.OUTSET = PIN7_bm;
    _delay_us(KENWOOD_SHORT_PULSE);
    PORTA.OUTCLR = PIN7_bm;
    _delay_ms(KENWOOD_MESSAGE_DELAY_mS);
}

void kenwood_output_byte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        (byte & 1) ? kenwood_binary_one() : kenwood_binary_zero();
        byte = (byte >> 1);
    }
}

void kenwood_output_swc(uint8_t command) {
    kenwood_preamble();
    kenwood_output_byte(KENWOOD_ADDRESS);
    kenwood_output_byte(KENWOOD_ADDRESS_INVERTED);
    kenwood_output_byte(command);
    kenwood_output_byte(~command);
    kenwood_postamble();
}

void kenwood_volume_up(void) {
    kenwood_output_swc(KENWOOD_VOLUME_UP);
}

void kenwood_volume_down(void) {
    kenwood_output_swc(KENWOOD_VOLUME_DOWN);
}

void kenwood_on_button_short_press(void) {
    kenwood_output_swc(KENWOOD_PLAY_PAUSE);
}

void kenwood_on_button_double_press(void) {
    kenwood_output_swc(KENWOOD_NEXT_TRACK);
}

void kenwood_on_button_held(void) {
    kenwood_output_swc(KENWOOD_MUTE);
}