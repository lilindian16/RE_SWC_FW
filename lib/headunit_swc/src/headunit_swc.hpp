#pragma once

#include <Arduino.h>

typedef enum
{
  HEADUNIT_GENERIC_RESISTIVE = 0x01,
  HEADUNIT_JVC,
  HEADUNIT_KENWOOD,
  HEADUNIT_ALPINE,
  HEADUNIT_PIONEER,
  HEADUNIT_BRAND_ERROR,
} Headunit_Brand_t;

class Headunit_SWC
{
public:
  virtual ~Headunit_SWC(void);
  virtual void on_encoder_rotation(bool cw_rotation);
  virtual void on_button_short_press(void);
  virtual void on_button_double_press(void);
  virtual void on_button_held(void);
};
