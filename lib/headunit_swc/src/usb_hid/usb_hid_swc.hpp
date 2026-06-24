#pragma once

#include "headunit_swc.hpp"

class USB_HID_SWC : public Headunit_SWC {
public:
  void init_usb_hid_swc(Output_Mapping_t *mapping);
  void on_encoder_rotation(bool cw_rotation);
  void on_button_short_press(void);
  void on_button_double_press(void);
  void on_button_held(void);

private:
  Output_Mapping_t *output_mapping;
  void update_consumer_report_descriptor(Output_Mapping_t *mapping);
  void usb_hid_output_swc(Volume_Knob_Input_t input);
};