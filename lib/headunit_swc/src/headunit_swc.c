#include "headunit_swc.h"
#include "headunits/generic_resistive/generic_resistive_headunit.h"
#include "headunits/jvc/jvc_headunit.h"
#include "headunits/kenwood/kenwood_headunit.h"

static Headunit_Brand_t* current_headunit_brand;

void init_headunit_swc(Headunit_Brand_t* headunit_brand) {
    current_headunit_brand = headunit_brand;
}

void headunit_volume_up(bool hold_output) {
    switch (*current_headunit_brand) {
        case HEADUNIT_GENERIC_RESISTIVE:
            generic_resistive_headunit_volume_up(hold_output);
            break;
        case HEADUNIT_JVC:
            jvc_volume_up();
            break;
        case HEADUNIT_KENWOOD:
            kenwood_volume_up();
            break;
        default:
            break;
    }
}

void headunit_volume_down(bool hold_output) {
    switch (*current_headunit_brand) {
        case HEADUNIT_GENERIC_RESISTIVE:
            generic_resistive_headunit_volume_down(hold_output);
            break;
        case HEADUNIT_JVC:
            jvc_volume_down();
            break;
        case HEADUNIT_KENWOOD:
            kenwood_volume_down();
            break;
        default:
            break;
    }
}

void headunit_button_short_press(bool hold_output) {
    switch (*current_headunit_brand) {
        case HEADUNIT_GENERIC_RESISTIVE:
            generic_resistive_headunit_button_short_press(hold_output);
            break;
        case HEADUNIT_JVC:
            jvc_on_button_short_press();
            break;
        case HEADUNIT_KENWOOD:
            kenwood_on_button_short_press();
            break;
        default:
            break;
    }
}

void headunit_button_held(bool hold_output) {
    switch (*current_headunit_brand) {
        case HEADUNIT_GENERIC_RESISTIVE:
            generic_resistive_headunit_button_held(hold_output);
            break;
        case HEADUNIT_JVC:
            jvc_on_button_held();
            break;
        case HEADUNIT_KENWOOD:
            kenwood_on_button_held();
            break;
        default:
            break;
    }
}

void headunit_button_double_pressed(bool hold_output) {
    switch (*current_headunit_brand) {
        case HEADUNIT_GENERIC_RESISTIVE:
            generic_resistive_headunit_button_double_press(hold_output);
            break;
        case HEADUNIT_JVC:
            jvc_on_button_double_press();
            break;
        case HEADUNIT_KENWOOD:
            kenwood_on_button_double_press();
            break;
        default:
            break;
    }
}