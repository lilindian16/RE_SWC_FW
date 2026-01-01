#pragma once

#include "headunit_swc.hpp"

class USB_HID_SWC : public Headunit_SWC {
public:
  void init_usb_hid_swc(void);
  void on_encoder_rotation(bool cw_rotation);
  void on_button_short_press(void);
  void on_button_double_press(void);
  void on_button_held(void);

private:
  uint8_t _KB_Data_Pack[2] = {0x00}; // Media key report
  void    _send_keyboard_command(uint8_t command);
};