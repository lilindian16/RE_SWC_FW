#pragma once

#include "headunit_swc.hpp"
#include <mcp4131.hpp>

class Pioneer_SWC : public Headunit_SWC {
public:
  void init_pioneer_swc(MCP4131 *mcp4131_ptr, int swc_gnd_en_pin,
                        Output_Mapping_t *mapping);
  void on_encoder_rotation(bool cw_rotation);
  void on_button_short_press(void);
  void on_button_double_press(void);
  void on_button_held(void);

private:
  int               _swc_gnd_enable_pin = -1;
  Output_Mapping_t *output_mapping;
  MCP4131          *_mcp4131;

  void pioneer_output_swc(Output_Command_t swc_command);
};