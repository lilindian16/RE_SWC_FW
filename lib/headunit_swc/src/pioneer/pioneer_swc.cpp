#include "pioneer_swc.hpp"

#include <Arduino.h>
#include <mcp4131.hpp>

#define OUTPUT_DELAY_MS 50

#define VOLUME_UP_RESISTANCE_OHMS 16000
#define VOLUME_DOWN_RESISTANCE_OHMS 24000
#define MUTE_RESISTANCE_OHMS 3500
#define NEXT_TRACK_RESISTANCE_OHMS 8000
#define PREVIOUS_TRACK_RESISTANCE_OHMS 11250

#define BUTTON_SHORT_PRESS_RESISTANCE_OHMS MUTE_RESISTANCE_OHMS
#define BUTTON_HELD_RESISTANCE_OHMS NEXT_TRACK_RESISTANCE_OHMS
#define BUTTON_DOUBLE_PRESS_RESISTANCE_OHMS PREVIOUS_TRACK_RESISTANCE_OHMS

void Pioneer_SWC::init_pioneer_swc(MCP4131 *mcp4131_ptr, int swc_gnd_en_pin)
{
    if (mcp4131_ptr)
    {
        this->_mcp4131 = mcp4131_ptr;
        this->_mcp4131->set_output_resistance(0x00);
        this->_swc_gnd_enable_pin = swc_gnd_en_pin;
        pinMode(this->_swc_gnd_enable_pin, OUTPUT);
        digitalWrite(this->_swc_gnd_enable_pin, LOW);
    }
    else
    {
        while (1)
        {
            ;
        }
    }
}

void Pioneer_SWC::on_encoder_rotation(bool clockwise_rotation)
{
    uint32_t required_resistance = (clockwise_rotation) ? VOLUME_UP_RESISTANCE_OHMS : VOLUME_DOWN_RESISTANCE_OHMS;
    this->_mcp4131->set_output_resistance(required_resistance);
    digitalWrite(this->_swc_gnd_enable_pin, HIGH);
    delay(OUTPUT_DELAY_MS);
    digitalWrite(this->_swc_gnd_enable_pin, LOW);
    delay(OUTPUT_DELAY_MS);
}

void Pioneer_SWC::on_button_short_press(void)
{
    uint32_t required_resistance = BUTTON_SHORT_PRESS_RESISTANCE_OHMS;
    this->_mcp4131->set_output_resistance(required_resistance);
    digitalWrite(this->_swc_gnd_enable_pin, HIGH);
    delay(OUTPUT_DELAY_MS);
    digitalWrite(this->_swc_gnd_enable_pin, LOW);
    delay(OUTPUT_DELAY_MS);
}

void Pioneer_SWC::on_button_double_press(void)
{
    uint32_t required_resistance = BUTTON_DOUBLE_PRESS_RESISTANCE_OHMS;
    this->_mcp4131->set_output_resistance(required_resistance);
    digitalWrite(this->_swc_gnd_enable_pin, HIGH);
    delay(OUTPUT_DELAY_MS);
    digitalWrite(this->_swc_gnd_enable_pin, LOW);
    delay(OUTPUT_DELAY_MS);
}

void Pioneer_SWC::on_button_held(void)
{
    uint32_t required_resistance = BUTTON_HELD_RESISTANCE_OHMS;
    this->_mcp4131->set_output_resistance(required_resistance);
    digitalWrite(this->_swc_gnd_enable_pin, HIGH);
    delay(OUTPUT_DELAY_MS);
    digitalWrite(this->_swc_gnd_enable_pin, LOW);
    delay(OUTPUT_DELAY_MS);
}