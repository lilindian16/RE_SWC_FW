#include "testing.hpp"

#include <Arduino.h>

void Testing::init_testing(MCP4131 *mcp4131_ptr, int mcp4131_cs_pin,
                           int swc_gnd_en_pin, int alpine_in_out_pin,
                           Alpine_SWC            *alpine_swc,
                           Generic_Resistive_SWC *generic_resistive_swc,
                           JVC_SWC *jvc_swc, Kenwood_SWC *kenwood_swc,
                           Pioneer_SWC *pioneer_swc, USB_HID_SWC *usb_hid_swc) {
  this->_mcp4131            = mcp4131_ptr;
  this->_mcp4131_cs_pin     = mcp4131_cs_pin;
  this->_swc_gnd_enable_pin = swc_gnd_en_pin;
  this->_alpine_in_out_pin  = alpine_in_out_pin;

  this->_alpine_swc            = alpine_swc;
  this->_generic_resistive_swc = generic_resistive_swc;
  this->_jvc_swc               = jvc_swc;
  this->_kenwood_swc           = kenwood_swc;
  this->_pioneer_swc           = pioneer_swc;
  this->_usb_hid_swc           = usb_hid_swc;

  this->_mcp4131->init(&SPI, this->_mcp4131_cs_pin);
}

void Testing::on_button_short_press(void) {
  // Start the test procedure
  this->_test_alpine_output();
  pinMode(this->_alpine_in_out_pin, INPUT); // Make pin floating
  delay(100);
  this->_test_kenwood_output();
  delay(100);
  this->_test_generic_resistance_output();
  delay(100);
  this->_test_usb_output();
}

void Testing::on_encoder_rotation(bool cw_rotation) {}

void Testing::on_button_double_press(void) {}

void Testing::on_button_held(void) {}

void Testing::_test_alpine_output(void) {
  this->_alpine_swc->init_alpine_swc(this->_alpine_in_out_pin);
  this->_alpine_swc->on_encoder_rotation(true);
}

void Testing::_test_kenwood_output(void) {
  this->_kenwood_swc->init_kenwood_swc(this->_swc_gnd_enable_pin);
  this->_kenwood_swc->on_encoder_rotation(true);
}

void Testing::_test_generic_resistance_output(void) { return; }

void Testing::_test_usb_output(void) {
  this->_usb_hid_swc->init_usb_hid_swc();
  this->_usb_hid_swc->on_encoder_rotation(true);
}