#pragma once

#include "headunit_swc.hpp"
#include <mcp4131.hpp>

#include "alpine/alpine_swc.hpp"
#include "generic_resistive/generic_resistive_swc.hpp"
#include "jvc/jvc_swc.hpp"
#include "kenwood/kenwood_swc.hpp"
#include "pioneer/pioneer_swc.hpp"
#include "usb_hid/usb_hid_swc.hpp"

class Testing : public Headunit_SWC {
public:
  void init_testing(MCP4131 *mcp4131_ptr, int mcp4131_cs_pin,
                    int swc_gnd_en_pin, int alpine_in_out_pin,
                    Alpine_SWC            *alpine_swc,
                    Generic_Resistive_SWC *generic_resistive_swc,
                    JVC_SWC *jvc_swc, Kenwood_SWC *kenwood_swc,
                    Pioneer_SWC *pioneer_swc, USB_HID_SWC *usb_hid_swc);
  void on_encoder_rotation(bool cw_rotation);
  void on_button_short_press(void);
  void on_button_double_press(void);
  void on_button_held(void);

private:
  int _swc_gnd_enable_pin = -1;
  int _mcp4131_cs_pin     = -1;
  int _alpine_in_out_pin  = -1;

  Alpine_SWC            *_alpine_swc;
  Generic_Resistive_SWC *_generic_resistive_swc;
  JVC_SWC               *_jvc_swc;
  Kenwood_SWC           *_kenwood_swc;
  Pioneer_SWC           *_pioneer_swc;
  USB_HID_SWC           *_usb_hid_swc;

  void _test_alpine_output(void);
  void _test_kenwood_output(void);
  void _test_generic_resistance_output(void);
  void _test_usb_output(void);

  MCP4131 *_mcp4131;
};