#include "generic_resistive_swc.hpp"

#include <Arduino.h>
#include <mcp4131.hpp>

#define OUTPUT_DELAY_NOT_HELD_MS 80
#define OUTPUT_DELAY_HELD        4000

#define VOLUME_UP_RESISTANCE_OHMS      1000
#define VOLUME_DOWN_RESISTANCE_OHMS    2000
#define MUTE_RESISTANCE_OHMS           4000
#define NEXT_TRACK_RESISTANCE_OHMS     6000
#define PREVIOUS_TRACK_RESISTANCE_OHMS 8000

#define BUTTON_SHORT_PRESS_RESISTANCE_OHMS NEXT_TRACK_RESISTANCE_OHMS
#define BUTTON_HELD_RESISTANCE_OHMS        PREVIOUS_TRACK_RESISTANCE_OHMS

void Generic_Resistive_SWC::init_generic_resistive_swc(MCP4131 *mcp4131_ptr,
                                                       int swc_gnd_en_pin) {
  if (mcp4131_ptr) {
    this->_mcp4131 = mcp4131_ptr;
    this->_mcp4131->set_output_resistance(0x00);
    this->_swc_gnd_enable_pin = swc_gnd_en_pin;
    pinMode(this->_swc_gnd_enable_pin, OUTPUT);
    digitalWrite(this->_swc_gnd_enable_pin, LOW);
  } else {
    while (1) {
      ;
    }
  }
}

void Generic_Resistive_SWC::on_encoder_rotation(bool clockwise_rotation) {
  uint32_t required_resistance = (clockwise_rotation)
                                     ? VOLUME_UP_RESISTANCE_OHMS
                                     : VOLUME_DOWN_RESISTANCE_OHMS;
  this->_mcp4131->set_output_resistance(required_resistance);
  digitalWrite(this->_swc_gnd_enable_pin, HIGH);
  if (this->_current_learning_mode_state == WAITING) {
    delay(OUTPUT_DELAY_HELD);
    this->_current_learning_mode_state = COMPLETE;
  } else {

    delay(OUTPUT_DELAY_NOT_HELD_MS);
  }
  digitalWrite(this->_swc_gnd_enable_pin, LOW);
}

void Generic_Resistive_SWC::on_button_short_press(void) {
  uint32_t required_resistance = BUTTON_SHORT_PRESS_RESISTANCE_OHMS;
  this->_mcp4131->set_output_resistance(required_resistance);
  digitalWrite(this->_swc_gnd_enable_pin, HIGH);
  if (this->_current_learning_mode_state == WAITING) {
    delay(OUTPUT_DELAY_HELD);
    this->_current_learning_mode_state = COMPLETE;
  } else {
    delay(OUTPUT_DELAY_NOT_HELD_MS);
  }
  digitalWrite(this->_swc_gnd_enable_pin, LOW);
}

void Generic_Resistive_SWC::on_button_double_press(void) {
  this->_current_learning_mode_state = WAITING;
}

void Generic_Resistive_SWC::on_learning_mode_completed(void) {
  this->_current_learning_mode_state = IDLE;
}

void Generic_Resistive_SWC::on_button_held(void) {
  uint32_t required_resistance = BUTTON_HELD_RESISTANCE_OHMS;
  this->_mcp4131->set_output_resistance(required_resistance);
  digitalWrite(this->_swc_gnd_enable_pin, HIGH);
  if (this->_current_learning_mode_state == WAITING) {
    delay(OUTPUT_DELAY_HELD);
    this->_current_learning_mode_state = COMPLETE;
  } else {
    delay(OUTPUT_DELAY_NOT_HELD_MS);
  }
  digitalWrite(this->_swc_gnd_enable_pin, LOW);
}

Learning_Mode_State_t Generic_Resistive_SWC::get_learning_mode_state(void) {
  return (this->_current_learning_mode_state);
}

void Generic_Resistive_SWC::run_loop_test(void) {
  while (true) {
    uint32_t required_resistances[] = {0, 3500, 8000, 11250, 16000, 24000};
    for (uint8_t i = 0;
         i < sizeof(required_resistances) / sizeof(required_resistances[0]);
         i++) {
      this->_mcp4131->set_output_resistance(required_resistances[i]);
      digitalWrite(this->_swc_gnd_enable_pin, HIGH);
      delay(10000);
    }
  }
}