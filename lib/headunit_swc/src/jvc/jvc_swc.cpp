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
#define JVC_PREAMBLE_AGC_PULSE_LENGTH_mS  9
#define JVC_PREAMBLE_LONG_PAUSE_LENGTH_mS 4
#define JVC_MESSAGE_TRANMISSION_GAP_mS    7
#define JVC_MIN_SPACE_BETWEEN_WORDS_mS    46

enum JVC_SWC_Command {
  JVC_COMMAND_VOLUME_UP      = 0x84,
  JVC_COMMAND_VOLUME_DOWN    = 0x85,
  JVC_COMMAND_MUTE           = 0x8E,
  JVC_COMMAND_NEXT_TRACK     = 0x92,
  JVC_COMMAND_PREVIOUS_TRACK = 0x93,
  JVC_COMMAND_UNKNOWN        = 0xFF
};

void JVC_SWC::init_jvc_swc(int gnd_en_pin, Output_Mapping_t *mapping) {
  this->_gnd_en_pin = gnd_en_pin;
  pinMode(this->_gnd_en_pin, OUTPUT);
  digitalWrite(this->_gnd_en_pin, LOW);
  this->output_mapping = mapping;
}

void JVC_SWC::on_encoder_rotation(bool cw_rotation) {
  this->jvc_output_swc((cw_rotation) ? this->output_mapping->cw_rotate_output
                                     : this->output_mapping->ccw_rotate_output);
}

void JVC_SWC::on_button_short_press(void) {
  this->jvc_output_swc(this->output_mapping->button_short_press_output);
}

void JVC_SWC::on_button_double_press(void) {
  this->jvc_output_swc(this->output_mapping->button_double_press_output);
}

void JVC_SWC::on_button_held(void) {
  this->jvc_output_swc(this->output_mapping->button_long_press_output);
}

void JVC_SWC::jvc_output_swc(Output_Command_t swc_command) {

  uint8_t jvc_command;

  switch (swc_command) {
  case VOLUME_UP:
    jvc_command = JVC_COMMAND_VOLUME_UP;
    break;

  case VOLUME_DOWN:
    jvc_command = JVC_COMMAND_VOLUME_DOWN;
    break;

  case MUTE:
    jvc_command = JVC_COMMAND_MUTE;
    break;

  case NEXT_TRACK:
    jvc_command = JVC_COMMAND_NEXT_TRACK;
    break;

  case PREVIOUS_TRACK:
    jvc_command = JVC_COMMAND_PREVIOUS_TRACK;
    break;

  case PLAY_PAUSE:
    jvc_command = JVC_COMMAND_UNKNOWN;
    break;

  case CHANGE_SOURCE:
    jvc_command = JVC_COMMAND_UNKNOWN;
    break;

  default:
    jvc_command = JVC_COMMAND_UNKNOWN;
    break;
  }

  if (jvc_command != JVC_COMMAND_UNKNOWN) {
    if (millis() - this->_previous_message_timestamp >= 60 ||
        jvc_command != this->_previous_command) {
      /* Set timestamp to the start of the message */
      this->_previous_message_timestamp = millis();
      /*
      The body of the message must be sent twice when it is a new command.
      Older models require this to confirm the change of command. Send the
      command now and wait to send the second command after
      */
      jvc_message_preamble();
      write_byte_out(JVC_DEVICE_ADDRESS);
      write_byte_out(jvc_command);
      jvc_message_postamble();
    }

    while (millis() - this->_previous_message_timestamp <=
           JVC_MIN_SPACE_BETWEEN_WORDS_mS) {
      delay(1);
    }
    /* Set timestamp to the start of the message */
    this->_previous_message_timestamp = millis();
    write_byte_out(JVC_DEVICE_ADDRESS);
    write_byte_out(jvc_command);
    jvc_message_postamble();
    this->_previous_command = jvc_command;
  }
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
  delay(JVC_PREAMBLE_AGC_PULSE_LENGTH_mS);
  digitalWrite(this->_gnd_en_pin, LOW);
  delay(JVC_PREAMBLE_LONG_PAUSE_LENGTH_mS);
}

void JVC_SWC::jvc_message_postamble(void) {
  jvc_binary_one(); // Stop bit 1
  delay(JVC_MESSAGE_TRANMISSION_GAP_mS);
}