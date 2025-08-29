#ifndef GENERIC_RESISTIVE_HEADUNIT_H
#define GENERIC_RESISTIVE_HEADUNIT_H

#include <stdbool.h>

void generic_resistive_headunit_volume_up (bool hold_output);
void generic_resistive_headunit_volume_down (bool hold_output);
void generic_resistive_headunit_button_short_press(bool hold_output);
void generic_resistive_headunit_button_held(bool hold_output);
void generic_resistive_headunit_button_double_pressed(bool hold_output);

#endif