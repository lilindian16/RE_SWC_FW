#pragma once

#include <mcp4131.hpp>
#include "headunit_swc.hpp"

typedef enum
{
  IDLE,
  WAITING,
  COMPLETE,
} Learning_Mode_State_t;

class Generic_Resistive_SWC : public Headunit_SWC
{
public:
  void init_generic_resistive_swc(MCP4131 *mcp4131_ptr, int swc_gnd_en_pin);
  void on_encoder_rotation(bool cw_rotation);
  void on_button_short_press(void);
  void on_button_double_press(void);
  void on_button_held(void);

  Learning_Mode_State_t get_learning_mode_state(void);
  void on_learning_mode_completed(void);

  void run_loop_test(void);

private:
  int _swc_gnd_enable_pin = -1;
  Learning_Mode_State_t _current_learning_mode_state = IDLE;

  MCP4131 *_mcp4131;
};