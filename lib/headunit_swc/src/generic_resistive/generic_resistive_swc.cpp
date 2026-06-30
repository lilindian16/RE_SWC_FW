#include "generic_resistive_swc.hpp"

#include <Arduino.h>
#include <mcp4131.hpp>

#define OUTPUT_DELAY_NOT_HELD_mS       80
#define OUTPUT_DELAY_LEARNING_MODE_mS  4000
#define OUTPUT_DELAY_BETWEEN_FRAMES_mS 20

void Generic_Resistive_SWC::init_generic_resistive_swc(
    MCP4131 *mcp4131_ptr, int swc_gnd_en_pin, Output_Mapping_t *mapping) {

  this->_mcp4131 = mcp4131_ptr;
  this->_mcp4131->set_output_resistance(0x00);
  this->_swc_gnd_enable_pin = swc_gnd_en_pin;
  pinMode(this->_swc_gnd_enable_pin, OUTPUT);
  digitalWrite(this->_swc_gnd_enable_pin, LOW);
  this->output_mapping = mapping;
}

void Generic_Resistive_SWC::on_encoder_rotation(bool clockwise_rotation) {
  this->generic_resistive_output_swc(
      (clockwise_rotation)
          ? this->output_mapping->cw_rotate_resistance_output
          : this->output_mapping->ccw_rotate_resistance_output);
}

void Generic_Resistive_SWC::on_button_short_press(void) {
  this->generic_resistive_output_swc(
      this->output_mapping->button_short_press_resistance_output);
}

void Generic_Resistive_SWC::on_button_held(void) {
  this->generic_resistive_output_swc(
      this->output_mapping->button_long_press_resistance_output);
}

void Generic_Resistive_SWC::on_button_double_press(void) {
  this->_current_learning_mode_state = WAITING;
}

void Generic_Resistive_SWC::on_learning_mode_completed(void) {
  this->_current_learning_mode_state = IDLE;
}

Learning_Mode_State_t Generic_Resistive_SWC::get_learning_mode_state(void) {
  return (this->_current_learning_mode_state);
}

void Generic_Resistive_SWC::generic_resistive_output_swc(
    uint8_t ladder_output_value) {
  this->_mcp4131->set_output_ladder_value(ladder_output_value);
  digitalWrite(this->_swc_gnd_enable_pin, HIGH);
  if (this->_current_learning_mode_state == WAITING) {
    delay(OUTPUT_DELAY_LEARNING_MODE_mS);
    this->_current_learning_mode_state = COMPLETE;
  } else {
    delay(this->output_mapping->resistance_output_hold_delay_time_units * 10);
  }
  digitalWrite(this->_swc_gnd_enable_pin, LOW);
  delay(this->output_mapping->resistance_output_off_delay_time_units * 10);
}