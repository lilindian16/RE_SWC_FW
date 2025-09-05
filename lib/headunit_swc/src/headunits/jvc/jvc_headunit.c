/**
 * JVC steering wheel control interface
 * Refer to img/ folder for logic analyser captures showing
 * how this protocol was reverse engineered
 */

#include "jvc_headunit.h"

#include <avr/io.h>
#include <util/delay.h>

/* Define the tick resolution in micro-seconds. This is the time required for a single bit of data .. hard to explain,
 * easier to show in the logic analyser captures */
#define JVC_TICK_RESOLUTION_uS            530
#define JVC_DATA_LENGTH_BITS              7
#define JVC_PREAMBLE_AGC_PULSE_LENGTH_mS  9
#define JVC_PREAMBLE_LONG_PAUSE_LENGTH_ms 4
#define JVC_MESSAGE_TRANMISSION_GAP_mS    7
#define JVC_DATA_MAX_VALUE                0x7F // 7-bit
#define JVC_HEADUNIT_ADDRESS              0x47
#define JVC_REPEATED_MESSAGE_REPITITIONS  1
#define JVC_NEW_MESSAGE_REPITITIONS       3
#define JVC_MESSAGE_REPEAT_DELAY_mS       9
#define JVC_MESSAGE_DELAY_mS              175

enum JVC_SWC_Command {
    JVC_VOLUME_UP_COMMAND = 0x04,
    JVC_VOLUME_DOWN_COMMAND = 0x05,
    JVC_MUTE_COMMAND = 0x0E,
    JVC_NEXT_TRACK = 0x12,
    JVC_PREVIOUS_TRACK = 0x13,
    JVC_COMMAND_UNKNOWN = 0xFF
};

/* Global variables */
static uint8_t previous_command = JVC_COMMAND_UNKNOWN;

/* Write a binary zero. We are controlling an open-drain output so we must invert the signals */
void jvc_binary_zero(void) {
    PORTA.OUTSET = PIN7_bm;
    _delay_us(JVC_TICK_RESOLUTION_uS);
    PORTA.OUTCLR = PIN7_bm;
    _delay_us(JVC_TICK_RESOLUTION_uS);
}

/* Write a binary one. We are controlling an open-drain output so we must invert the signals */
void jvc_binary_one(void) {
    PORTA.OUTSET = PIN7_bm;
    _delay_us(JVC_TICK_RESOLUTION_uS);
    PORTA.OUTCLR = PIN7_bm;
    _delay_us(JVC_TICK_RESOLUTION_uS * 3);
}

void jvc_message_preamble(void) {
    PORTA.OUTSET = PIN7_bm;
    _delay_ms(JVC_PREAMBLE_AGC_PULSE_LENGTH_mS);
    PORTA.OUTCLR = PIN7_bm;
    _delay_ms(JVC_PREAMBLE_LONG_PAUSE_LENGTH_ms);
    jvc_binary_one(); // Start bit
    /* Write JVC Address */
    jvc_binary_one();
    jvc_binary_one();
    jvc_binary_one();
    jvc_binary_zero();
    jvc_binary_zero();
    jvc_binary_zero();
    jvc_binary_one();
}

void jvc_message_postamble(void) {
    jvc_binary_one(); // Stop bit 1
    jvc_binary_one(); // Stop bit 2
    _delay_ms(JVC_MESSAGE_TRANMISSION_GAP_mS);
}

void jvc_output_swc(uint8_t swc_command) {
    uint8_t message_repitions =
        (previous_command == swc_command) ? JVC_REPEATED_MESSAGE_REPITITIONS : JVC_NEW_MESSAGE_REPITITIONS;
    previous_command = swc_command;
    while (message_repitions) {
        jvc_message_preamble();
        /* Create a temp copy so we can quickly bit-shift the value out */
        uint8_t temp_swc_command = swc_command;
        for (uint8_t i = 0; i < JVC_DATA_LENGTH_BITS; i++) {
            (temp_swc_command & 1) ? jvc_binary_one() : jvc_binary_zero();
            temp_swc_command = (temp_swc_command >> 1);
        }
        jvc_message_postamble();
        message_repitions--;
        _delay_ms(JVC_MESSAGE_REPEAT_DELAY_mS);
    }
    if (swc_command == JVC_VOLUME_UP_COMMAND) {
        _delay_ms(JVC_MESSAGE_DELAY_mS);
    }
}

void jvc_volume_up(void) {
    jvc_output_swc(JVC_VOLUME_UP_COMMAND);
}

void jvc_volume_down(void) {
    jvc_output_swc(JVC_VOLUME_DOWN_COMMAND);
}

void jvc_on_button_short_press(void) {
    jvc_output_swc(JVC_MUTE_COMMAND);
}

void jvc_on_button_double_press(void) {
    jvc_output_swc(JVC_NEXT_TRACK);
}

void jvc_on_button_held(void) {
    jvc_output_swc(JVC_PREVIOUS_TRACK);
}
