#pragma once

#include <Arduino.h>

typedef enum : uint8_t {
  HEADUNIT_GENERIC_RESISTIVE = 0x01,
  HEADUNIT_JVC               = 0x02,
  HEADUNIT_KENWOOD           = 0x03,
  HEADUNIT_ALPINE            = 0x04,
  HEADUNIT_PIONEER           = 0x05,
  HEADUNIT_USB_HID           = 0x06,
  HEADUNIT_SONY              = 0x07,
  HEADUNIT_BRAND_ERROR,
} Headunit_Brand_t;

typedef enum : uint8_t {
  VOLUME_KNOB_INPUT_CW_ROTATION,
  VOLUME_KNOB_INPUT_CCW_ROTATION,
  VOLUME_KNOB_INPUT_BUTTON_SHORT_PRESS,
  VOLUME_KNOB_INPUT_BUTTON_LONG_PRESS,
  VOLUME_KNOB_INPUT_BUTTON_DOUBLE_PRESS,
} Volume_Knob_Input_t;

typedef enum : uint8_t {
  NOT_CONFIGURED = 0x00,
  VOLUME_UP,
  VOLUME_DOWN,
  MUTE,
  NEXT_TRACK,
  PREVIOUS_TRACK,
  PLAY_PAUSE,
  CHANGE_SOURCE,
} Output_Command_t;

typedef struct {
  Headunit_Brand_t headunit_brand;
  Output_Command_t cw_rotate_output;
  Output_Command_t ccw_rotate_output;
  Output_Command_t button_short_press_output;
  Output_Command_t button_long_press_output;
  Output_Command_t button_double_press_output;
  uint8_t          cw_rotate_resistance_output;
  uint8_t          ccw_rotate_resistance_output;
  uint8_t          button_short_press_resistance_output;
  uint8_t          button_long_press_resistance_output;
  uint8_t          button_double_press_resistance_output;
  uint8_t          resistance_output_hold_delay_time_units; // each unit is 10ms
  uint8_t          resistance_output_off_delay_time_units;  // each unit is 10ms
} Output_Mapping_t;

class Headunit_SWC {
public:
  virtual ~Headunit_SWC(void);
  virtual void on_encoder_rotation(bool cw_rotation);
  virtual void on_button_short_press(void);
  virtual void on_button_double_press(void);
  virtual void on_button_held(void);
};
