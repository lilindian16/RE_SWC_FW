#pragma once

#include "headunit_swc.hpp"

/*
According to the protocol, all commands are sent LSB first so these enums
reflect that
*/
enum JVC_SWC_Command {
  JVC_VOLUME_UP_COMMAND   = 0x84,
  JVC_VOLUME_DOWN_COMMAND = 0x85,
  JVC_MUTE_COMMAND        = 0x8E,
  JVC_NEXT_TRACK          = 0x92,
  JVC_PREVIOUS_TRACK      = 0x93,
  JVC_COMMAND_UNKNOWN     = 0xFF
};

class JVC_SWC : public Headunit_SWC {
public:
  void init_jvc_swc(int gnd_en_pin);
  void on_encoder_rotation(bool cw_rotation);
  void on_button_short_press(void);
  void on_button_double_press(void);
  void on_button_held(void);

private:
  int      _gnd_en_pin;
  uint8_t  _previous_command           = JVC_COMMAND_UNKNOWN;
  uint16_t _previous_message_timestamp = 0;
  uint16_t _current_message_timestamp  = 0;

  void jvc_output_swc(uint8_t swc_command);
  void write_byte_out(uint8_t output_byte);
  void jvc_binary_zero(void);
  void jvc_binary_one(void);
  void jvc_message_preamble(void);
  void jvc_message_postamble(void);
};