#pragma once

#include "headunit_swc.hpp"

typedef enum {
  ALPINE_MUTE       = 0x16,
  ALPINE_VOL_UP     = 0x14,
  ALPINE_VOL_DOWN   = 0x15,
  ALPINE_NEXT_TRACK = 0x12,
  ALPINE_PREV_TRACK = 0x13,
} Alpine_Command_t;

class Alpine_SWC : public Headunit_SWC {
public:
  void init_alpine_swc(int alpine_output_pin);
  void on_encoder_rotation(bool cw_rotation);
  void on_button_short_press(void);
  void on_button_double_press(void);
  void on_button_held(void);

private:
  int _alpine_output_pin;

  void write_swc_command(Alpine_Command_t command);
  void output_byte(uint8_t data);
  void alpine_binary_zero(void);
  void alpine_binary_one(void);
};