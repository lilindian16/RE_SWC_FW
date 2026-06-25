/**
 * Kenwood steering wheel control interface
 * Refer to img/ folder for logic analyser captures showing
 * how this protocol was reverse engineered
 */

#include "kenwood_swc.hpp"
#include "headunit_swc.hpp"

/* Define the tick resolution in micro-seconds. This is the time required for a
 * single bit of data .. hard to explain, easier to show in the logic analyser
 * captures */
#define KENWOOD_TICK_RESOLUTION_uS               530
#define KENWOOD_SHORT_PULSE                      KENWOOD_TICK_RESOLUTION_uS
#define KENWOOD_LONG_PULSE                       (KENWOOD_TICK_RESOLUTION_uS * 3)
#define KENWOOD_PREAMBLE_LONG_PULSE_DURATION_mS  9
#define KENWOOD_PREAMBLE_PULSE_PAUSE_DURATION_mS 4
#define KENWOOD_MESSAGE_DELAY_mS                 5

#define KENWOOD_ADDRESS          0xB9
#define KENWOOD_ADDRESS_INVERTED 0x46

enum Kenwood_SWC_Command {
  KENWOOD_COMMAND_VOLUME_UP      = 0x14,
  KENWOOD_COMMAND_VOLUME_DOWN    = 0x15,
  KENWOOD_COMMAND_MUTE           = 0x16,
  KENWOOD_COMMAND_NEXT_TRACK     = 0x0B,
  KENWOOD_COMMAND_PREVIOUS_TRACK = 0x0A,
  KENWOOD_COMMAND_PLAY_PAUSE     = 0x0E,
  KENWOOD_COMMAND_UNKNOWN        = 0xff,
};

void Kenwood_SWC::init_kenwood_swc(int               gnd_control_pin,
                                   Output_Mapping_t *mapping) {
  this->_gnd_control_pin = gnd_control_pin;
  pinMode(this->_gnd_control_pin, OUTPUT);
  digitalWrite(this->_gnd_control_pin, LOW);
  this->output_mapping = mapping;
}

void Kenwood_SWC::on_encoder_rotation(bool cw_rotation) {

  this->kenwood_output_swc((cw_rotation)
                               ? this->output_mapping->cw_rotate_output
                               : this->output_mapping->ccw_rotate_output);
}

void Kenwood_SWC::on_button_short_press(void) {
  this->kenwood_output_swc(this->output_mapping->button_short_press_output);
}

void Kenwood_SWC::on_button_double_press(void) {
  this->kenwood_output_swc(this->output_mapping->button_double_press_output);
}

void Kenwood_SWC::on_button_held(void) {
  this->kenwood_output_swc(this->output_mapping->button_long_press_output);
}

void Kenwood_SWC::kenwood_output_swc(Output_Command_t swc_command) {

  uint8_t kenwood_command;
  switch (swc_command) {
  case VOLUME_UP:
    kenwood_command = KENWOOD_COMMAND_VOLUME_UP;
    break;

  case VOLUME_DOWN:
    kenwood_command = KENWOOD_COMMAND_VOLUME_DOWN;
    break;

  case MUTE:
    kenwood_command = KENWOOD_COMMAND_MUTE;
    break;

  case NEXT_TRACK:
    kenwood_command = KENWOOD_COMMAND_NEXT_TRACK;
    break;

  case PREVIOUS_TRACK:
    kenwood_command = KENWOOD_COMMAND_PREVIOUS_TRACK;
    break;

  case PLAY_PAUSE:
    kenwood_command = KENWOOD_COMMAND_PLAY_PAUSE;
    break;

  case CHANGE_SOURCE:
    kenwood_command = KENWOOD_COMMAND_UNKNOWN;
    break;

  default:
    kenwood_command = KENWOOD_COMMAND_UNKNOWN;
    break;
  }

  if (kenwood_command != KENWOOD_COMMAND_UNKNOWN) {
    kenwood_preamble();
    kenwood_output_byte(KENWOOD_ADDRESS);
    kenwood_output_byte(KENWOOD_ADDRESS_INVERTED);
    kenwood_output_byte(kenwood_command);
    kenwood_output_byte(~kenwood_command);
    kenwood_postamble();
  }
}

void Kenwood_SWC::kenwood_binary_one(void) {
  digitalWrite(this->_gnd_control_pin, HIGH);
  delayMicroseconds(KENWOOD_SHORT_PULSE);
  digitalWrite(this->_gnd_control_pin, LOW);
  delayMicroseconds(KENWOOD_LONG_PULSE);
}

void Kenwood_SWC::kenwood_binary_zero(void) {
  digitalWrite(this->_gnd_control_pin, HIGH);
  delayMicroseconds(KENWOOD_SHORT_PULSE);
  digitalWrite(this->_gnd_control_pin, LOW);
  delayMicroseconds(KENWOOD_SHORT_PULSE);
}

void Kenwood_SWC::kenwood_preamble(void) {
  digitalWrite(this->_gnd_control_pin, HIGH);
  delay(KENWOOD_PREAMBLE_LONG_PULSE_DURATION_mS);
  digitalWrite(this->_gnd_control_pin, LOW);
  delay(KENWOOD_PREAMBLE_PULSE_PAUSE_DURATION_mS);
}

void Kenwood_SWC::kenwood_postamble(void) {
  digitalWrite(this->_gnd_control_pin, HIGH);
  delayMicroseconds(KENWOOD_SHORT_PULSE);
  digitalWrite(this->_gnd_control_pin, LOW);
  delay(KENWOOD_MESSAGE_DELAY_mS);
}

void Kenwood_SWC::kenwood_output_byte(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    (data & 1) ? kenwood_binary_one() : kenwood_binary_zero();
    data = (data >> 1);
  }
}
