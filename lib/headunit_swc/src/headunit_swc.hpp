#pragma once

#include <Arduino.h>

typedef enum {
  HEADUNIT_GENERIC_RESISTIVE = 0x01,
  HEADUNIT_JVC               = 0x02,
  HEADUNIT_KENWOOD           = 0x03,
  HEADUNIT_ALPINE            = 0x04,
  HEADUNIT_PIONEER           = 0x05,
  HEADUNIT_USB_HID           = 0x06,
  HEADUNIT_SONY              = 0x07,
  SWC_TESTING                = 0x08,
  HEADUNIT_BRAND_ERROR,
} Headunit_Brand_t;

class Headunit_SWC {
public:
  virtual ~Headunit_SWC(void);
  virtual void on_encoder_rotation(bool cw_rotation);
  virtual void on_button_short_press(void);
  virtual void on_button_double_press(void);
  virtual void on_button_held(void);
};
