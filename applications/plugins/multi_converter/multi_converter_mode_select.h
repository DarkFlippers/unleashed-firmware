#pragma once

#include <stdlib.h>
#include <input/input.h>
#include <gui/gui.h>

#include "multi_converter_definitions.h"
#include "multi_converter_units.h"

//
// aux draw function for units offsets and draw stuff
//
void multi_converter_mode_select_draw_destination_offset(
    uint8_t x,
    uint8_t y,
    int8_t d,
    Canvas* const canvas,
    const MultiConverterState* multi_converter_state);

void multi_converter_mode_select_draw_selected_unit(
    uint8_t x,
    uint8_t y,
    MultiConverterUnitType unit_type,
    Canvas* const canvas);

//
// draw the main SELECT view with the current multi_converter_state values
//
void multi_converter_mode_select_draw(
    Canvas* const canvas,
    const MultiConverterState* multi_converter_state);

//
// reset the SELECT mode view, showing as "pre-selected" the current working units
//
void multi_converter_mode_select_reset(MultiConverterState* const multi_converter_state);

//
// exit from SELECT mode and go back to display view, if save_changes == 1 use the current SELECT view info
// to modify the current selected units and reset the views properly (usually if the ORIGIN unit has been
// changed, reset everything; otherwise just trigger the convert function with a new DESTINATION)
//
// currently this function DON'T CHECK invalid unit relations (the navigation and display functions will
// prevent weird behaviours, so for now we're trusting the selected_unit_orig/dest_type values)
//
// returns an enum code MultiConverterDisplayTrigger based on doing nothing (cancel), triggering the display
// convert method or reseting the whole display mode (when fully changing the units)
//
// notice the MODE CHANGE itself is not done here but in the main loop (outside the call) via the ModeTrigger enum element
//
MultiConverterModeTrigger multi_converter_mode_select_exit(
    uint8_t save_changes,
    MultiConverterState* const multi_converter_state);

//
// switch between selecting the ORIGIN or the DESTINATION unit on DISPLAY mode (since there're only
// two options, both left/right arrow keys acts as toggles, no "direction" required)
//
void multi_converter_mode_select_switch(MultiConverterState* const multi_converter_state);

//
// change the selected unit on SELECTED mode, using the select_orig flag to check if we're switching the
// ORIGIN or the DESTINATION unit; the DIRECTION (up or down to travel the array) is set as a param
//
// when switching the ORIGIN one, reset the DESTINATION to the first valid unit (if the current one is not
// valid anymore); when switching the DESTINATION one, an allowed_function() check is performed in order to
// properly set a valid destination unit.
//
// (notice the draw step also perform which units are valid to display, so no worries about that here)
//
void multi_converter_mode_select_change_unit(
    int8_t direction,
    MultiConverterState* const multi_converter_state);
