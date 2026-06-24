#pragma once

#include "headunit_swc.hpp"

/*
According to the protocol, all commands are sent LSB first so these enums
reflect that
*/

class JVC_SWC : public Headunit_SWC {
public:
  void init_jvc_swc(int gnd_en_pin, Output_Mapping_t *mapping);
  void on_encoder_rotation(bool cw_rotation);
  void on_button_short_press(void);
  void on_button_double_press(void);
  void on_button_held(void);

private:
  int               _gnd_en_pin;
  uint8_t           _previous_command           = 0;
  uint16_t          _previous_message_timestamp = 0;
  uint16_t          _current_message_timestamp  = 0;
  Output_Mapping_t *output_mapping;

  void jvc_output_swc(Output_Command_t swc_command);
  void write_byte_out(uint8_t output_byte);
  void jvc_binary_zero(void);
  void jvc_binary_one(void);
  void jvc_message_preamble(void);
  void jvc_message_postamble(void);
};