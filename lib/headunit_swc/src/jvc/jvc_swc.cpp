/**
 * JVC steering wheel control interface
 * Refer to img/ folder for logic analyser captures showing
 * how this protocol was reverse engineered
 */

#include "jvc_swc.hpp"

/* Define the tick resolution in micro-seconds. This is the time required for a single bit of data .. hard to explain,
 * easier to show in the logic analyser captures */
#define JVC_TICK_RESOLUTION_uS 530
#define JVC_DATA_LENGTH_BITS 7
#define JVC_PREAMBLE_AGC_PULSE_LENGTH_mS 9
#define JVC_PREAMBLE_LONG_PAUSE_LENGTH_ms 4
#define JVC_MESSAGE_TRANMISSION_GAP_mS 7
#define JVC_DATA_MAX_VALUE 0x7F // 7-bit
#define JVC_HEADUNIT_ADDRESS 0x47
#define JVC_REPEATED_MESSAGE_REPITITIONS 1
#define JVC_NEW_MESSAGE_REPITITIONS 3
#define JVC_MESSAGE_REPEATdelay 9
#define JVC_MESSAGEdelay 175

void JVC_SWC::init_jvc_swc(int gnd_en_pin)
{
    this->_gnd_en_pin = gnd_en_pin;
    pinMode(this->_gnd_en_pin, OUTPUT);
    digitalWrite(this->_gnd_en_pin, LOW);
}

void JVC_SWC::on_encoder_rotation(bool cw_rotation)
{
    if (cw_rotation)
    {
        this->jvc_output_swc(JVC_VOLUME_UP_COMMAND);
    }
    else
    {
        this->jvc_output_swc(JVC_VOLUME_DOWN_COMMAND);
    }
}

void JVC_SWC::on_button_short_press(void)
{
    this->jvc_output_swc(JVC_MUTE_COMMAND);
}

void JVC_SWC::on_button_double_press(void)
{
    this->jvc_output_swc(JVC_PREVIOUS_TRACK);
}

void JVC_SWC::on_button_held(void)
{
    this->jvc_output_swc(JVC_NEXT_TRACK);
}

void JVC_SWC::jvc_output_swc(uint8_t swc_command)
{
    uint8_t message_repitions =
        (this->_previous_command == swc_command) ? JVC_REPEATED_MESSAGE_REPITITIONS : JVC_NEW_MESSAGE_REPITITIONS;
    this->_previous_command = swc_command;
    while (message_repitions)
    {
        jvc_message_preamble();
        /* Create a temp copy so we can quickly bit-shift the value out */
        uint8_t temp_swc_command = swc_command;
        for (uint8_t i = 0; i < JVC_DATA_LENGTH_BITS; i++)
        {
            (temp_swc_command & 1) ? jvc_binary_one() : jvc_binary_zero();
            temp_swc_command = (temp_swc_command >> 1);
        }
        jvc_message_postamble();
        message_repitions--;
        delay(JVC_MESSAGE_REPEATdelay);
    }
}

/* Write a binary zero. We are controlling an open-drain output so we must invert the signals */
void JVC_SWC::jvc_binary_zero(void)
{
    digitalWrite(this->_gnd_en_pin, HIGH);
    delayMicroseconds(JVC_TICK_RESOLUTION_uS);
    digitalWrite(this->_gnd_en_pin, LOW);
    delayMicroseconds(JVC_TICK_RESOLUTION_uS);
}

/* Write a binary one. We are controlling an open-drain output so we must invert the signals */
void JVC_SWC::jvc_binary_one(void)
{
    digitalWrite(this->_gnd_en_pin, HIGH);
    delayMicroseconds(JVC_TICK_RESOLUTION_uS);
    digitalWrite(this->_gnd_en_pin, LOW);
    delayMicroseconds(JVC_TICK_RESOLUTION_uS * 3);
}

void JVC_SWC::jvc_message_preamble(void)
{
    digitalWrite(this->_gnd_en_pin, HIGH);
    delay(JVC_PREAMBLE_AGC_PULSE_LENGTH_mS);
    digitalWrite(this->_gnd_en_pin, LOW);
    delay(JVC_PREAMBLE_LONG_PAUSE_LENGTH_ms);
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

void JVC_SWC::jvc_message_postamble(void)
{
    jvc_binary_one(); // Stop bit 1
    jvc_binary_one(); // Stop bit 2
    delay(JVC_MESSAGE_TRANMISSION_GAP_mS);
}