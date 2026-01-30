/**
 * JVC steering wheel control interface
 * Refer to img/ folder for logic analyser captures showing
 * how this protocol was reverse engineered
 *
 * https://support.jvc.com/consumer/support/documents/RemoteCodes.pdf
 */

#include "jvc_swc.hpp"

#define JVC_DEVICE_ADDRESS                0x8F
#define JVC_TICK_RESOLUTION_uS            530
#define JVC_DATA_LENGTH_BITS              8
#define JVC_PREAMBLE_AGC_PULSE_LENGTH_MS  9
#define JVC_PREAMBLE_LONG_PAUSE_LENGTH_MS 4
#define JVC_MESSAGE_TRANMISSION_GAP_MS    7
#define JVC_REPEATED_MESSAGE_REPITITIONS  1
#define JVC_NEW_MESSAGE_REPITITIONS       0
#define JVC_MESSAGE_REPEAT_DELAY_MS       9
#define JVC_MIN_SPACE_BETWEEN_WORDDS_MS   46
#define JVC_MAX_SPACE_BETWEEN_WORDS_MS    60

void JVC_SWC::init_jvc_swc(int gnd_en_pin) {
  this->_gnd_en_pin = gnd_en_pin;
  pinMode(this->_gnd_en_pin, OUTPUT);
  digitalWrite(this->_gnd_en_pin, LOW);
}

void JVC_SWC::on_encoder_rotation(bool cw_rotation) {
  if (cw_rotation) {
    this->jvc_output_swc(JVC_VOLUME_UP_COMMAND);
  } else {
    this->jvc_output_swc(JVC_VOLUME_DOWN_COMMAND);
  }
}

void JVC_SWC::on_button_short_press(void) {
  this->jvc_output_swc(JVC_MUTE_COMMAND);
}

void JVC_SWC::on_button_double_press(void) {
  this->jvc_output_swc(JVC_PREVIOUS_TRACK);
}

void JVC_SWC::on_button_held(void) { this->jvc_output_swc(JVC_NEXT_TRACK); }

void JVC_SWC::jvc_output_swc(uint8_t swc_command) {

  if (millis() - this->_previous_message_timestamp >= 60 ||
      swc_command != this->_previous_command) {
    /* Set timestamp to the start of the message */
    this->_previous_message_timestamp = millis();
    /*
    The body of the message must be sent twice when it is a new command.
    Older models require this to confirm the change of command. Send the command
    now and wait to send the second command after
    */
    jvc_message_preamble();
    write_byte_out(JVC_DEVICE_ADDRESS);
    write_byte_out(swc_command);
    jvc_message_postamble();
  }

  while (millis() - this->_previous_message_timestamp <=
         JVC_MIN_SPACE_BETWEEN_WORDDS_MS) {
    delay(1);
  }
  /* Set timestamp to the start of the message */
  this->_previous_message_timestamp = millis();
  write_byte_out(JVC_DEVICE_ADDRESS);
  write_byte_out(swc_command);
  jvc_message_postamble();
  this->_previous_command = swc_command;
}

void JVC_SWC::write_byte_out(uint8_t output_byte) {
  for (uint8_t i = 0; i < JVC_DATA_LENGTH_BITS; i++) {
    (output_byte & 1) ? jvc_binary_one() : jvc_binary_zero();
    output_byte = (output_byte >> 1);
  }
}

/* Write a binary zero. We are controlling an open-drain output so we must
 * invert the signals */
void JVC_SWC::jvc_binary_zero(void) {
  digitalWrite(this->_gnd_en_pin, HIGH);
  delayMicroseconds(JVC_TICK_RESOLUTION_uS);
  digitalWrite(this->_gnd_en_pin, LOW);
  delayMicroseconds(JVC_TICK_RESOLUTION_uS);
}

/* Write a binary one. We are controlling an open-drain output so we must
 * invert the signals */
void JVC_SWC::jvc_binary_one(void) {
  digitalWrite(this->_gnd_en_pin, HIGH);
  delayMicroseconds(JVC_TICK_RESOLUTION_uS);
  digitalWrite(this->_gnd_en_pin, LOW);
  delayMicroseconds(JVC_TICK_RESOLUTION_uS * 3);
}

void JVC_SWC::jvc_message_preamble(void) {
  digitalWrite(this->_gnd_en_pin, HIGH);
  delay(JVC_PREAMBLE_AGC_PULSE_LENGTH_MS);
  digitalWrite(this->_gnd_en_pin, LOW);
  delay(JVC_PREAMBLE_LONG_PAUSE_LENGTH_MS);
}

void JVC_SWC::jvc_message_postamble(void) {
  jvc_binary_one(); // Stop bit 1
  delay(JVC_MESSAGE_TRANMISSION_GAP_MS);
}