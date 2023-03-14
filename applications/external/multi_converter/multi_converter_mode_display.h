#pragma once

#include <input/input.h>
#include <gui/gui.h>

#include "multi_converter_definitions.h"
#include "multi_converter_units.h"

//
// performs a unit conversion from origin to source buffers, if there's any error, overflow or
// non-compatible format (which shouldn't happen, but just in case) abort conversion and outputs
// some "?" strings on the buffer or something similar
//
void multi_converter_mode_display_convert(MultiConverterState* const multi_converter_state);

//
// draw the main DISPLAY view with the current multi_converter_state values
//
void multi_converter_mode_display_draw(
    Canvas* const canvas,
    const MultiConverterState* multi_converter_state);

//
// keyboard navigation on DISPLAY mode (NAVIGATION only, no BACK nor OK - InputKey guaranteed to be left/right/up/down)
//
void multi_converter_mode_display_navigation(
    InputKey key,
    MultiConverterState* const multi_converter_state);

//
// reset the DISPLAY mode with the current units, cleaning the buffers and different flags;
// call this when exiting the SELECT mode / changing the units
//
void multi_converter_mode_display_reset(MultiConverterState* const multi_converter_state);

//
// toggle the negative flag on current selected buffer ONLY if the unit allows negative numbers
// (adding negative number may crop the last char on the buffer; it cannot be recovered)
//
void multi_converter_mode_display_toggle_negative(MultiConverterState* const multi_converter_state);

//
// add a comma/dot/decimal separator/whatever on current selected buffer ONLY if the unit allows it
// (only ONE comma allowed, not in the beginning nor end)
//
void multi_converter_mode_display_add_comma(MultiConverterState* const multi_converter_state);

//
// add a regular number to the buffer if it's <= the max_number_keys from the unit (not necessary
// since the draw and navigation functions won't allow a trigger for an invalid number, but still
// to keep the "checks" policy on each "add key" function...)
//
void multi_converter_mode_display_add_number(MultiConverterState* const multi_converter_state);

//
// handle the OK action when selecting a specific key on the keyboard (add a number, a symbol, change mode...)
// returns a ModeTrigger enum value: may or may not let to a mode change on the main loop (WON'T change the mode here)
//
MultiConverterModeTrigger multi_converter_mode_display_ok(
    uint8_t long_press,
    MultiConverterState* const multi_converter_state);