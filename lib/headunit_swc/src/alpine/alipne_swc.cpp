#include "alpine_swc.hpp"

#include <Arduino.h>

#define ALPINE_BIT_RESOLUTION_US  540
#define ALPINE_ADDRESS            0x8672
#define DELAY_BETWEEN_MESSAGES_MS 30

void Alpine_SWC::init_alpine_swc(int alpine_output_pin) {
  _alpine_output_pin = alpine_output_pin;
  pinMode(_alpine_output_pin, OUTPUT);
  digitalWrite(_alpine_output_pin, LOW);
}

void Alpine_SWC::on_encoder_rotation(bool cw_rotation) {
  this->write_swc_command((cw_rotation) ? ALPINE_VOL_UP : ALPINE_VOL_DOWN);
}

void Alpine_SWC::on_button_short_press(void) {
  this->write_swc_command(ALPINE_MUTE);
}

void Alpine_SWC::on_button_double_press(void) {
  this->write_swc_command(ALPINE_PREV_TRACK);
}

void Alpine_SWC::on_button_held(void) {
  this->write_swc_command(ALPINE_NEXT_TRACK);
}

void Alpine_SWC::write_swc_command(Alpine_Command_t command) {
  /* Start with SOF */
  digitalWrite(this->_alpine_output_pin, HIGH);
  delay(9);
  digitalWrite(this->_alpine_output_pin, LOW);
  delayMicroseconds(4500);
  /* Send the Address LSB first */
  this->output_byte(ALPINE_ADDRESS >> 8);
  this->output_byte(ALPINE_ADDRESS & 0xFF);
  this->output_byte(command);
  this->output_byte(~command);
  digitalWrite(this->_alpine_output_pin, HIGH);
  delayMicroseconds(ALPINE_BIT_RESOLUTION_US);
  digitalWrite(this->_alpine_output_pin, LOW);
  delayMicroseconds(ALPINE_BIT_RESOLUTION_US);
  delay(DELAY_BETWEEN_MESSAGES_MS);
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