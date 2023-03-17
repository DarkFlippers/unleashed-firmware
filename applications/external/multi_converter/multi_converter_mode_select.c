#include "multi_converter_mode_select.h"

#define MULTI_CONVERTER_LIST_ENTRIES_COUNT 3

#define MULTI_CONVERTER_INFO_STRING_FROM "FROM:"
#define MULTI_CONVERTER_INFO_STRING_TO "TO:"
#define MULTI_CONVERTER_INFO_STRING_OK "OK: Change"
#define MULTI_CONVERTER_INFO_STRING_BACK "BACK: Cancel"

void multi_converter_mode_select_draw_destination_offset(
    uint8_t x,
    uint8_t y,
    int8_t d,
    Canvas* const canvas,
    const MultiConverterState* multi_converter_state) {
    int i = 1;
    while(
        i <
        MULTI_CONVERTER_AVAILABLE_UNITS) { // in case there's no match, to avoid an endless loop (in theory shouldn't happen, but...)
        int ut = multi_converter_get_unit_type_offset(
            (multi_converter_state->select).selected_unit_type_dest, i * d);
        if(multi_converter_available_units[(multi_converter_state->select).selected_unit_type_orig]
               .allowed_function(ut) &&
           (multi_converter_state->select).selected_unit_type_orig != ut) {
            canvas_draw_str(canvas, x, y, multi_converter_available_units[ut].name);
            break;
        }
        i++;
    }
}

void multi_converter_mode_select_draw_selected_unit(
    uint8_t x,
    uint8_t y,
    MultiConverterUnitType unit_type,
    Canvas* const canvas) {
    canvas_draw_box(
        canvas,
        x - 2,
        y - 10,
        canvas_string_width(canvas, multi_converter_available_units[unit_type].name) + 4,
        13);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_str(canvas, x, y, multi_converter_available_units[unit_type].name);
    canvas_set_color(canvas, ColorBlack);
}

void multi_converter_mode_select_draw(
    Canvas* const canvas,
    const MultiConverterState* multi_converter_state) {
    int y = 10;
    int x = 10;

    canvas_set_color(canvas, ColorBlack);

    // FROM
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, x, y, MULTI_CONVERTER_INFO_STRING_FROM);

    canvas_set_font(canvas, FontSecondary);

    // offset -1
    y += 12;

    canvas_draw_str(
        canvas,
        x,
        y,
        multi_converter_available_units[multi_converter_get_unit_type_offset(
                                            (multi_converter_state->select).selected_unit_type_orig,
                                            -1)]
            .name);

    // current selected element
    y += 12;

    multi_converter_mode_select_draw_selected_unit(
        x, y, (multi_converter_state->select).selected_unit_type_orig, canvas);

    if((multi_converter_state->select).select_orig) canvas_draw_str(canvas, x - 6, y, ">");

    // offset +1
    y += 12;

    canvas_draw_str(
        canvas,
        x,
        y,
        multi_converter_available_units[multi_converter_get_unit_type_offset(
                                            (multi_converter_state->select).selected_unit_type_orig,
                                            1)]
            .name);

    // TO
    y = 10;
    x = 70;

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, x, y, MULTI_CONVERTER_INFO_STRING_TO);

    canvas_set_font(canvas, FontSecondary);

    // offset -1: go back from current selected destination and find the first one valid (even if it's itself)
    y += 12;

    multi_converter_mode_select_draw_destination_offset(x, y, -1, canvas, multi_converter_state);

    // current selected element
    y += 12;

    multi_converter_mode_select_draw_selected_unit(
        x, y, (multi_converter_state->select).selected_unit_type_dest, canvas);

    if(!(multi_converter_state->select).select_orig) canvas_draw_str(canvas, x - 6, y, ">");

    // offset +1: same but on the opposite direction
    y += 12;

    multi_converter_mode_select_draw_destination_offset(x, y, 1, canvas, multi_converter_state);

    // OK / CANCEL

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(
        canvas, 0, 64 - 12, canvas_string_width(canvas, MULTI_CONVERTER_INFO_STRING_OK) + 4, 12);
    canvas_draw_box(
        canvas,
        128 - 4 - canvas_string_width(canvas, MULTI_CONVERTER_INFO_STRING_BACK),
        64 - 12,
        canvas_string_width(canvas, "BACK: Cancel") + 4,
        12);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_str(canvas, 2, 64 - 3, MULTI_CONVERTER_INFO_STRING_OK);
    canvas_draw_str(
        canvas,
        128 - 2 - canvas_string_width(canvas, MULTI_CONVERTER_INFO_STRING_BACK),
        64 - 3,
        MULTI_CONVERTER_INFO_STRING_BACK);
}

void multi_converter_mode_select_reset(MultiConverterState* const multi_converter_state) {
    // initial pre-selected values are equal to the current selected values
    (multi_converter_state->select).selected_unit_type_orig =
        multi_converter_state->unit_type_orig;
    (multi_converter_state->select).selected_unit_type_dest =
        multi_converter_state->unit_type_dest;

    (multi_converter_state->select).select_orig = 1;
}

MultiConverterModeTrigger multi_converter_mode_select_exit(
    uint8_t save_changes,
    MultiConverterState* const multi_converter_state) {
    if(save_changes) {
        multi_converter_state->unit_type_dest =
            (multi_converter_state->select).selected_unit_type_dest;

        if(multi_converter_state->unit_type_orig ==
           (multi_converter_state->select).selected_unit_type_orig) {
            // if the ORIGIN unit didn't changed, just trigger the convert

            return Convert;
        } else {
            multi_converter_state->unit_type_orig =
                (multi_converter_state->select).selected_unit_type_orig;
            multi_converter_state->unit_type_dest =
                (multi_converter_state->select).selected_unit_type_dest;

            return Reset;
        }
    }

    return None;
}

void multi_converter_mode_select_switch(MultiConverterState* const multi_converter_state) {
    (multi_converter_state->select).select_orig ^= 1;
}

void multi_converter_mode_select_change_unit(
    int8_t direction,
    MultiConverterState* const multi_converter_state) {
    MultiConverterUnitType d;
    if((multi_converter_state->select).select_orig) {
        (multi_converter_state->select).selected_unit_type_orig =
            multi_converter_get_unit_type_offset(
                (multi_converter_state->select).selected_unit_type_orig, direction);
        d = (multi_converter_state->select).selected_unit_type_dest;
    } else {
        d = ((multi_converter_state->select).selected_unit_type_dest + direction) %
            MULTI_CONVERTER_AVAILABLE_UNITS;
    }

    // check each unit with the ORIGIN allowed_function() to make sure we're selecting a valid DESTINATION
    // (when changing the ORIGIN unit the DIRECTION in which we'll switch the DESTINATION will be the SAME);
    // also notice that ORIGIN must be DIFFERENT than DESTINATION
    int i = 0;
    while(i < MULTI_CONVERTER_AVAILABLE_UNITS) {
        if(multi_converter_available_units[(multi_converter_state->select).selected_unit_type_orig]
               .allowed_function(d) &&
           (multi_converter_state->select).selected_unit_type_orig != d) {
            (multi_converter_state->select).selected_unit_type_dest = d;
            break;
        }

        d = multi_converter_get_unit_type_offset(d, direction);
        i++;
    }
}
