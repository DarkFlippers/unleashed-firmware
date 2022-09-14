#include "multi_converter_mode_display.h"

#define MULTI_CONVERTER_DISPLAY_KEYS 18 // [0] to [F] + [BACK] + [SELECT]

#define MULTI_CONVERTER_DISPLAY_KEY_NEGATIVE 0 // long press
#define MULTI_CONVERTER_DISPLAY_KEY_COMMA 1 // long press
#define MULTI_CONVERTER_DISPLAY_KEY_DEL 16
#define MULTI_CONVERTER_DISPLAY_KEY_SELECT 17

#define MULTI_CONVERTER_DISPLAY_CHAR_COMMA '.'
#define MULTI_CONVERTER_DISPLAY_CHAR_NEGATIVE '-'
#define MULTI_CONVERTER_DISPLAY_CHAR_DEL '<'
#define MULTI_CONVERTER_DISPLAY_CHAR_SELECT '#'
#define MULTI_CONVERTER_DISPLAY_CHAR_BLANK ' '

#define MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN 3
#define MULTI_CONVERTER_DISPLAY_KEY_CHAR_HEIGHT 8

void multi_converter_mode_display_convert(MultiConverterState* const multi_converter_state) {
    // 1.- if origin == destination (in theory user won't be allowed to choose the same options, but it's kinda "valid"...)
    // just copy buffer_orig to buffer_dest and that's it

    if(multi_converter_state->unit_type_orig == multi_converter_state->unit_type_dest) {
        memcpy(
            multi_converter_state->buffer_dest,
            multi_converter_state->buffer_orig,
            MULTI_CONVERTER_NUMBER_DIGITS);
        return;
    }

    // 2.- origin_buffer has not null functions
    if(multi_converter_get_unit(multi_converter_state->unit_type_orig).convert_function == NULL ||
       multi_converter_get_unit(multi_converter_state->unit_type_orig).allowed_function == NULL)
        return;

    // 3.- valid destination type (using allowed_destinations function)
    if(!multi_converter_get_unit(multi_converter_state->unit_type_orig)
            .allowed_function(multi_converter_state->unit_type_dest))
        return;

    multi_converter_get_unit(multi_converter_state->unit_type_orig)
        .convert_function(multi_converter_state);
}

void multi_converter_mode_display_draw(
    Canvas* const canvas,
    const MultiConverterState* multi_converter_state) {
    canvas_set_color(canvas, ColorBlack);

    // ORIGIN
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(
        canvas, 2, 10, multi_converter_get_unit(multi_converter_state->unit_type_orig).mini_name);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2 + 30, 10, multi_converter_state->buffer_orig);

    // DESTINATION
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(
        canvas,
        2,
        10 + 12,
        multi_converter_get_unit(multi_converter_state->unit_type_dest).mini_name);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2 + 30, 10 + 12, multi_converter_state->buffer_dest);

    // SEPARATOR_LINE
    canvas_draw_line(canvas, 2, 25, 128 - 3, 25);

    // KEYBOARD
    uint8_t _x = 5;
    uint8_t _y = 25 + 15; // line + 10

    for(int i = 0; i < MULTI_CONVERTER_DISPLAY_KEYS; i++) {
        char g;
        if(i < 10)
            g = (i + '0');
        else if(i < 16)
            g = ((i - 10) + 'A');
        else if(i == MULTI_CONVERTER_DISPLAY_KEY_DEL)
            g = MULTI_CONVERTER_DISPLAY_CHAR_DEL;
        else
            g = MULTI_CONVERTER_DISPLAY_CHAR_SELECT;

        uint8_t g_w = canvas_glyph_width(canvas, g);

        if(i < 16 &&
           i > multi_converter_get_unit(multi_converter_state->unit_type_orig).max_number_keys -
                   1) {
            // some units won't use the full [0] - [F] keyboard, in those situations just hide the char
            // (won't be selectable anyway, so no worries here; this is just about drawing stuff)
            g = MULTI_CONVERTER_DISPLAY_CHAR_BLANK;
        }

        // currently hover key is highlighted
        if((multi_converter_state->display).key == i) {
            canvas_draw_box(
                canvas,
                _x - MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN,
                _y - (MULTI_CONVERTER_DISPLAY_KEY_CHAR_HEIGHT +
                      MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN),
                MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN + g_w +
                    MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN,
                MULTI_CONVERTER_DISPLAY_KEY_CHAR_HEIGHT +
                    MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN * 2);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_draw_frame(
                canvas,
                _x - MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN,
                _y - (MULTI_CONVERTER_DISPLAY_KEY_CHAR_HEIGHT +
                      MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN),
                MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN + g_w +
                    MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN,
                MULTI_CONVERTER_DISPLAY_KEY_CHAR_HEIGHT +
                    MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN * 2);
        }

        // draw key
        canvas_draw_glyph(canvas, _x, _y, g);

        // certain keys have long_press features, draw whatever they're using there too
        if(i == MULTI_CONVERTER_DISPLAY_KEY_NEGATIVE) {
            canvas_draw_box(
                canvas,
                _x + MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN + g_w - 4,
                _y + MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN - 2,
                4,
                2);
        } else if(i == MULTI_CONVERTER_DISPLAY_KEY_COMMA) {
            canvas_draw_box(
                canvas,
                _x + MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN + g_w - 2,
                _y + MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN - 2,
                2,
                2);
        }

        // back to black
        canvas_set_color(canvas, ColorBlack);

        if(i < 8) {
            _x += g_w + MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN * 2 + 2;
        } else if(i == 8) {
            _y += (MULTI_CONVERTER_DISPLAY_KEY_CHAR_HEIGHT +
                   MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN * 2) +
                  3;
            _x = 8; // some padding at the beginning on second line
        } else {
            _x += g_w + MULTI_CONVERTER_DISPLAY_KEY_FRAME_MARGIN * 2 + 1;
        }
    }
}

void multi_converter_mode_display_navigation(
    InputKey key,
    MultiConverterState* const multi_converter_state) {
    // first move to keyboard position, then check if the ORIGIN allows that specific key, if not jump to the "closest one"
    switch(key) {
    default:
        break;

    case InputKeyUp:
    case InputKeyDown:
        if((multi_converter_state->display).key >= 9)
            (multi_converter_state->display).key -= 9;
        else
            (multi_converter_state->display).key += 9;
        break;

    case InputKeyLeft:
    case InputKeyRight:

        (multi_converter_state->display).key += (key == InputKeyLeft ? -1 : 1);

        if((multi_converter_state->display).key > MULTI_CONVERTER_DISPLAY_KEYS - 1)
            (multi_converter_state->display).key = 0;
        else if((multi_converter_state->display).key < 0)
            (multi_converter_state->display).key = MULTI_CONVERTER_DISPLAY_KEYS - 1;
        break;
    }

    // if destination key is disabled by max_number_keys, move to the closest one
    // (this could be improved with more accurate keys movements, probably...)
    if(multi_converter_get_unit(multi_converter_state->unit_type_orig).max_number_keys >= 16)
        return; // weird, since this means "do not show any number on the keyboard, but just in case..."

    int8_t i = -1;
    if(key == InputKeyRight || key == InputKeyDown) i = 1;

    while((multi_converter_state->display).key < 16 &&
          (multi_converter_state->display).key >
              multi_converter_get_unit(multi_converter_state->unit_type_orig).max_number_keys -
                  1) {
        (multi_converter_state->display).key += i;
        if((multi_converter_state->display).key > MULTI_CONVERTER_DISPLAY_KEYS - 1)
            (multi_converter_state->display).key = 0;
        else if((multi_converter_state->display).key < 0)
            (multi_converter_state->display).key = MULTI_CONVERTER_DISPLAY_KEYS - 1;
    }
}

void multi_converter_mode_display_reset(MultiConverterState* const multi_converter_state) {
    // clean the buffers
    for(int i = 0; i < MULTI_CONVERTER_NUMBER_DIGITS; i++) {
        multi_converter_state->buffer_orig[i] = MULTI_CONVERTER_DISPLAY_CHAR_BLANK;
        multi_converter_state->buffer_dest[i] = MULTI_CONVERTER_DISPLAY_CHAR_BLANK;
    }

    // reset the display flags and index
    multi_converter_state->display.cursor = 0;
    multi_converter_state->display.key = 0;
    multi_converter_state->display.comma = 0;
    multi_converter_state->display.negative = 0;
}

void multi_converter_mode_display_toggle_negative(
    MultiConverterState* const multi_converter_state) {
    if(multi_converter_get_unit(multi_converter_state->unit_type_orig).allow_negative) {
        if(!(multi_converter_state->display).negative) {
            // shift origin buffer one to right + add the "-" sign (last digit will be lost)
            for(int i = MULTI_CONVERTER_NUMBER_DIGITS - 1; i > 0; i--) {
                // we could avoid the blanks, but nevermind
                multi_converter_state->buffer_orig[i] = multi_converter_state->buffer_orig[i - 1];
            }
            multi_converter_state->buffer_orig[0] = MULTI_CONVERTER_DISPLAY_CHAR_NEGATIVE;

            // only increment cursor if we're not out of bound
            if((multi_converter_state->display).cursor < MULTI_CONVERTER_NUMBER_DIGITS)
                (multi_converter_state->display).cursor++;
        } else {
            // shift origin buffer one to left, append ' ' on the end
            for(int i = 0; i < MULTI_CONVERTER_NUMBER_DIGITS - 1; i++) {
                if(multi_converter_state->buffer_orig[i] == MULTI_CONVERTER_DISPLAY_CHAR_BLANK)
                    break;

                multi_converter_state->buffer_orig[i] = multi_converter_state->buffer_orig[i + 1];
            }
            multi_converter_state->buffer_orig[MULTI_CONVERTER_NUMBER_DIGITS - 1] =
                MULTI_CONVERTER_DISPLAY_CHAR_BLANK;

            (multi_converter_state->display).cursor--;
        }

        // toggle flag
        (multi_converter_state->display).negative ^= 1;
    }
}

void multi_converter_mode_display_add_comma(MultiConverterState* const multi_converter_state) {
    if(!multi_converter_get_unit(multi_converter_state->unit_type_orig).allow_comma ||
       (multi_converter_state->display).comma || !(multi_converter_state->display).cursor ||
       ((multi_converter_state->display).cursor == (MULTI_CONVERTER_NUMBER_DIGITS - 1)))
        return; // maybe not allowerd; or one comma already in place; also cannot add commas as first or last chars

    // set flag to one
    (multi_converter_state->display).comma = 1;

    multi_converter_state->buffer_orig[(multi_converter_state->display).cursor] =
        MULTI_CONVERTER_DISPLAY_CHAR_COMMA;
    (multi_converter_state->display).cursor++;
}

void multi_converter_mode_display_add_number(MultiConverterState* const multi_converter_state) {
    if((multi_converter_state->display).key >
       multi_converter_get_unit(multi_converter_state->unit_type_orig).max_number_keys - 1)
        return;

    if((multi_converter_state->display).key < 10) {
        multi_converter_state->buffer_orig[(multi_converter_state->display).cursor] =
            (multi_converter_state->display).key + '0';
    } else {
        multi_converter_state->buffer_orig[(multi_converter_state->display).cursor] =
            ((multi_converter_state->display).key - 10) + 'A';
    }

    (multi_converter_state->display).cursor++;
}

MultiConverterModeTrigger multi_converter_mode_display_ok(
    uint8_t long_press,
    MultiConverterState* const multi_converter_state) {
    if((multi_converter_state->display).key < MULTI_CONVERTER_DISPLAY_KEY_DEL) {
        if((multi_converter_state->display).cursor >= MULTI_CONVERTER_NUMBER_DIGITS)
            return None; // limit reached, ignore

        // long press on 0 toggle NEGATIVE if allowed, on 1 adds COMMA if allowed
        if(long_press) {
            if((multi_converter_state->display).key == MULTI_CONVERTER_DISPLAY_KEY_NEGATIVE) {
                // toggle negative
                multi_converter_mode_display_toggle_negative(multi_converter_state);
            } else if((multi_converter_state->display).key == MULTI_CONVERTER_DISPLAY_KEY_COMMA) {
                // add comma
                multi_converter_mode_display_add_comma(multi_converter_state);
            }

        } else {
            // regular keys
            multi_converter_mode_display_add_number(multi_converter_state);
        }

        multi_converter_mode_display_convert(multi_converter_state);

    } else if((multi_converter_state->display).key == MULTI_CONVERTER_DISPLAY_KEY_DEL) {
        if((multi_converter_state->display).cursor > 0) (multi_converter_state->display).cursor--;

        if(multi_converter_state->buffer_orig[(multi_converter_state->display).cursor] ==
           MULTI_CONVERTER_DISPLAY_CHAR_COMMA)
            (multi_converter_state->display).comma = 0;
        if(multi_converter_state->buffer_orig[(multi_converter_state->display).cursor] ==
           MULTI_CONVERTER_DISPLAY_CHAR_NEGATIVE)
            (multi_converter_state->display).negative = 0;

        multi_converter_state->buffer_orig[(multi_converter_state->display).cursor] =
            MULTI_CONVERTER_DISPLAY_CHAR_BLANK;

        multi_converter_mode_display_convert(multi_converter_state);

    } else { // MULTI_CONVERTER_DISPLAY_KEY_SELECT
        return Reset;
    }

    return None;
}