#ifndef HEADUNIT_SWC_H
#define HEADUNIT_SWC_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HEADUNIT_BRAND_UNKNOWN = 0,
    HEADUNIT_GENERIC_RESISTIVE,
    HEADUNIT_JVC,
    HEADUNIT_KENWOOD,
    HEADUNIT_BRAND_ERROR,
}Headunit_Brand_t;

void init_headunit_swc(Headunit_Brand_t *headunit_brand);

void headunit_volume_up (bool hold_output);
void headunit_volume_down(bool hold_output);
void headunit_button_short_press(bool hold_output);
void headunit_button_held(bool hold_output);
void headunit_button_double_pressed(bool hold_output);

#endif