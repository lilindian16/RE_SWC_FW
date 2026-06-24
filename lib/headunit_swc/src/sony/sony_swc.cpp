#include "sony_swc.hpp"

#include <Arduino.h>
#include <mcp4131.hpp>

#define OUTPUT_DELAY_MS 50

#define SONY_VOLUME_UP_RESISTANCE_OHMS      16800
#define SONY_VOLUME_DOWN_RESISTANCE_OHMS    23600
#define SONY_MUTE_RESISTANCE_OHMS           4400
#define SONY_NEXT_TRACK_RESISTANCE_OHMS     8800
#define SONY_PREVIOUS_TRACK_RESISTANCE_OHMS 12100
#define SONY_UNKNOWN_OUTPUT_RESISTANCE_OHMS 0

void Sony_SWC::init_sony_swc(MCP4131 *mcp4131_ptr, int swc_gnd_en_pin,
                             Output_Mapping_t *mapping) {
  this->_mcp4131 = mcp4131_ptr;
  this->_mcp4131->set_output_resistance(0x00);
  this->_swc_gnd_enable_pin = swc_gnd_en_pin;
  pinMode(this->_swc_gnd_enable_pin, OUTPUT);
  digitalWrite(this->_swc_gnd_enable_pin, LOW);
  this->output_mapping = mapping;
}

void Sony_SWC::on_encoder_rotation(bool clockwise_rotation) {
  this->sony_output_swc((clockwise_rotation)
                            ? this->output_mapping->cw_rotate_output
                            : this->output_mapping->ccw_rotate_output);
}

void Sony_SWC::on_button_short_press(void) {
  this->sony_output_swc(this->output_mapping->button_short_press_output);
}

void Sony_SWC::on_button_double_press(void) {
  this->sony_output_swc(this->output_mapping->button_double_press_output);
}

void Sony_SWC::on_button_held(void) {
  this->sony_output_swc(this->output_mapping->button_long_press_output);
}

void Sony_SWC::sony_output_swc(Output_Command_t swc_command) {
  uint32_t required_resistance;
  switch (swc_command) {
  case VOLUME_UP:
    required_resistance = SONY_VOLUME_UP_RESISTANCE_OHMS;
    break;

  case VOLUME_DOWN:
    required_resistance = SONY_VOLUME_DOWN_RESISTANCE_OHMS;
    break;

  case MUTE:
    required_resistance = SONY_MUTE_RESISTANCE_OHMS;
    break;

  case NEXT_TRACK:
    required_resistance = SONY_NEXT_TRACK_RESISTANCE_OHMS;
    break;

  case PREVIOUS_TRACK:
    required_resistance = SONY_PREVIOUS_TRACK_RESISTANCE_OHMS;
    break;

  case PLAY_PAUSE:
    required_resistance = SONY_UNKNOWN_OUTPUT_RESISTANCE_OHMS;
    break;

  case CHANGE_SOURCE:
    required_resistance = SONY_UNKNOWN_OUTPUT_RESISTANCE_OHMS;
    break;

  default:
    required_resistance = SONY_UNKNOWN_OUTPUT_RESISTANCE_OHMS;
    break;
  }

  if (required_resistance != SONY_UNKNOWN_OUTPUT_RESISTANCE_OHMS) {
    this->_mcp4131->set_output_resistance(required_resistance);
    digitalWrite(this->_swc_gnd_enable_pin, HIGH);
    delay(OUTPUT_DELAY_MS);
    digitalWrite(this->_swc_gnd_enable_pin, LOW);
    delay(OUTPUT_DELAY_MS);
  }
}