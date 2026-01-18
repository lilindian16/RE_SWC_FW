/**
 * Kenwood steering wheel control interface
 * Refer to img/ folder for logic analyser captures showing
 * how this protocol was reverse engineered
 */

#include "kenwood_swc.hpp"

#include "headunit_swc.hpp"

#define KENWOOD_DATA_LENGTH_BITS 8
/* Define the tick resolution in micro-seconds. This is the time required for a
 * single bit of data .. hard to explain, easier to show in the logic analyser
 * captures */
#define KENWOOD_TICK_RESOLUTION_uS               530
#define KENWOOD_SHORT_PULSE                      KENWOOD_TICK_RESOLUTION_uS
#define KENWOOD_LONG_PULSE                       (KENWOOD_TICK_RESOLUTION_uS * 3)
#define KENWOOD_PREAMBLE_LONG_PULSE_DURATION_mS  9
#define KENWOOD_PREAMBLE_PULSE_PAUSE_DURATION_mS 4
#define KENWOOD_MESSAGEdelay                     5

#define KENWOOD_ADDRESS          0xB9
#define KENWOOD_ADDRESS_INVERTED 0x46

enum Kenwood_SWC_Command {
  KENWOOD_PREVIOUS_TRACK = 0x0A,
  KENWOOD_NEXT_TRACK     = 0x0B,
  KENWOOD_PLAY_PAUSE     = 0x0E,
  KENWOOD_VOLUME_UP      = 0x14,
  KENWOOD_VOLUME_DOWN    = 0x15,
  KENWOOD_MUTE           = 0x16,
};

void Kenwood_SWC::init_kenwood_swc(int gnd_control_pin) {
  this->_gnd_control_pin = gnd_control_pin;
  pinMode(this->_gnd_control_pin, OUTPUT);
  digitalWrite(this->_gnd_control_pin, LOW);
}

void Kenwood_SWC::on_encoder_rotation(bool cw_rotation) {
  if (cw_rotation) {
    this->kenwood_output_swc(KENWOOD_VOLUME_UP);
  }

  else {
    this->kenwood_output_swc(KENWOOD_VOLUME_DOWN);
  }
}

void Kenwood_SWC::on_button_short_press(void) {
  this->kenwood_output_swc(KENWOOD_MUTE);
}

void Kenwood_SWC::on_button_double_press(void) {
  this->kenwood_output_swc(KENWOOD_PREVIOUS_TRACK);
}

void Kenwood_SWC::on_button_held(void) {
  this->kenwood_output_swc(KENWOOD_NEXT_TRACK);
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
  delay(KENWOOD_MESSAGEdelay);
}

void Kenwood_SWC::kenwood_output_byte(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    (data & 1) ? kenwood_binary_one() : kenwood_binary_zero();
    data = (data >> 1);
  }
}

void Kenwood_SWC::kenwood_output_swc(uint8_t command) {
  kenwood_preamble();
  kenwood_output_byte(KENWOOD_ADDRESS);
  kenwood_output_byte(KENWOOD_ADDRESS_INVERTED);
  kenwood_output_byte(command);
  kenwood_output_byte(~command);
  kenwood_postamble();
}
