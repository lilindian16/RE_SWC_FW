#pragma once

#include "headunit_swc.hpp"

class Alpine_SWC : public Headunit_SWC {
public:
  void init_alpine_swc(int alpine_output_pin, Output_Mapping_t *mapping);
  void on_encoder_rotation(bool cw_rotation);
  void on_button_short_press(void);
  void on_button_double_press(void);
  void on_button_held(void);

private:
  int               _alpine_output_pin;
  Output_Mapping_t *output_mapping;

  void write_swc_command(Output_Command_t swc_command);
  void output_byte(uint8_t data);
  void alpine_binary_zero(void);
  void alpine_binary_one(void);
};