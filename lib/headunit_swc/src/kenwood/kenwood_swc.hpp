#pragma once

#include <headunit_swc.hpp>

class Kenwood_SWC : public Headunit_SWC {
public:
  void init_kenwood_swc(int gnd_control_pin);
  void on_encoder_rotation(bool cw_rotation);
  void on_button_short_press(void);
  void on_button_double_press(void);
  void on_button_held(void);

private:
  void kenwood_binary_one(void);
  void kenwood_binary_zero(void);
  void kenwood_preamble(void);
  void kenwood_postamble(void);
  void kenwood_output_byte(uint8_t data);
  void kenwood_output_swc(uint8_t command);

  int _gnd_control_pin = -1;
};