#include "alpine_swc.hpp"

#include <Arduino.h>

#define ALPINE_BIT_RESOLUTION_US  540
#define ALPINE_ADDRESS            0x8672
#define DELAY_BETWEEN_MESSAGES_MS 30

typedef enum {
  ALPINE_COMMAND_VOL_UP     = 0x14,
  ALPINE_COMMAND_VOL_DOWN   = 0x15,
  ALPINE_COMMAND_MUTE       = 0x16,
  ALPINE_COMMAND_NEXT_TRACK = 0x12,
  ALPINE_COMMAND_PREV_TRACK = 0x13,
  ALPINE_COMMAND_UNKNOWN    = 0xff,
} Alpine_Command_t;

void Alpine_SWC::init_alpine_swc(int               alpine_output_pin,
                                 Output_Mapping_t *mapping) {
  _alpine_output_pin = alpine_output_pin;
  pinMode(_alpine_output_pin, OUTPUT);
  digitalWrite(_alpine_output_pin, LOW);
  this->output_mapping = mapping;
}

void Alpine_SWC::on_encoder_rotation(bool cw_rotation) {
  this->write_swc_command((cw_rotation)
                              ? this->output_mapping->cw_rotate_output
                              : this->output_mapping->ccw_rotate_output);
}

void Alpine_SWC::on_button_short_press(void) {
  this->write_swc_command(this->output_mapping->button_short_press_output);
}

void Alpine_SWC::on_button_double_press(void) {
  this->write_swc_command(this->output_mapping->button_double_press_output);
}

void Alpine_SWC::on_button_held(void) {
  this->write_swc_command(this->output_mapping->button_long_press_output);
}

void Alpine_SWC::write_swc_command(Output_Command_t swc_command) {

  uint8_t alpine_command;
  switch (swc_command) {
  case VOLUME_UP:
    alpine_command = ALPINE_COMMAND_VOL_UP;
    break;

  case VOLUME_DOWN:
    alpine_command = ALPINE_COMMAND_VOL_DOWN;
    break;

  case MUTE:
    alpine_command = ALPINE_COMMAND_MUTE;
    break;

  case NEXT_TRACK:
    alpine_command = ALPINE_COMMAND_NEXT_TRACK;
    break;

  case PREVIOUS_TRACK:
    alpine_command = ALPINE_COMMAND_PREV_TRACK;
    break;

  case PLAY_PAUSE:
    alpine_command = ALPINE_COMMAND_UNKNOWN;
    break;

  case CHANGE_SOURCE:
    alpine_command = ALPINE_COMMAND_UNKNOWN;
    break;

  default:
    alpine_command = ALPINE_COMMAND_UNKNOWN;
    break;
  }

  if (alpine_command != ALPINE_COMMAND_UNKNOWN) {
    /* Start with SOF */
    digitalWrite(this->_alpine_output_pin, HIGH);
    delay(9);
    digitalWrite(this->_alpine_output_pin, LOW);
    delayMicroseconds(4500);
    /* Send the Address LSB first */
    this->output_byte(ALPINE_ADDRESS >> 8);
    this->output_byte(ALPINE_ADDRESS & 0xFF);
    this->output_byte(alpine_command);
    this->output_byte(~alpine_command);
    digitalWrite(this->_alpine_output_pin, HIGH);
    delayMicroseconds(ALPINE_BIT_RESOLUTION_US);
    digitalWrite(this->_alpine_output_pin, LOW);
    delayMicroseconds(ALPINE_BIT_RESOLUTION_US);
    delay(DELAY_BETWEEN_MESSAGES_MS);
  }
}

void Alpine_SWC::output_byte(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    (data & 1) ? alpine_binary_one() : alpine_binary_zero();
    data = (data >> 1);
  }
}

void Alpine_SWC::alpine_binary_zero(void) {
  digitalWrite(this->_alpine_output_pin, HIGH);
  delayMicroseconds(ALPINE_BIT_RESOLUTION_US);
  digitalWrite(this->_alpine_output_pin, LOW);
  delayMicroseconds(ALPINE_BIT_RESOLUTION_US);
}

void Alpine_SWC::alpine_binary_one(void) {
  digitalWrite(this->_alpine_output_pin, HIGH);
  delayMicroseconds(ALPINE_BIT_RESOLUTION_US);
  digitalWrite(this->_alpine_output_pin, LOW);
  delayMicroseconds(3 * ALPINE_BIT_RESOLUTION_US);
}