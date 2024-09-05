#include "number_input.h"

#include <gui/elements.h>
#include <furi.h>
#include <assets_icons.h>
#include <lib/toolbox/strint.h>

struct NumberInput {
    View* view;
};

typedef struct {
    const char text;
    const size_t x;
    const size_t y;
} NumberInputKey;

typedef struct {
    FuriString* header;
    FuriString* text_buffer;

    int32_t current_number;
    int32_t max_value;
    int32_t min_value;

    NumberInputCallback callback;
    void* callback_context;

    size_t selected_row;
    size_t selected_column;
} NumberInputModel;

static const size_t keyboard_origin_x = 7;
static const size_t keyboard_origin_y = 31;
static const size_t keyboard_row_count = 2;
static const char enter_symbol = '\r';
static const char backspace_symbol = '\b';
static const char sign_symbol = '-';

static const NumberInputKey keyboard_keys_row_1[] = {
    {'0', 0, 12},
    {'1', 11, 12},
    {'2', 22, 12},
    {'3', 33, 12},
    {'4', 44, 12},
    {backspace_symbol, 103, 4},
};

static const NumberInputKey keyboard_keys_row_2[] = {
    {'5', 0, 26},
    {'6', 11, 26},
    {'7', 22, 26},
    {'8', 33, 26},
    {'9', 44, 26},
    {sign_symbol, 55, 17},
    {enter_symbol, 95, 17},
};

static size_t number_input_get_row_size(size_t row_index) {
    size_t row_size = 0;

    switch(row_index + 1) {
    case 1:
        row_size = COUNT_OF(keyboard_keys_row_1);
        break;
    case 2:
        row_size = COUNT_OF(keyboard_keys_row_2);
        break;
    default:
        furi_crash();
    }

    return row_size;
}

static const NumberInputKey* number_input_get_row(size_t row_index) {
    const NumberInputKey* row = NULL;

    switch(row_index + 1) {
    case 1:
        row = keyboard_keys_row_1;
        break;
    case 2:
        row = keyboard_keys_row_2;
        break;
    default:
        furi_crash();
    }

    return row;
}

static void number_input_draw_input(Canvas* canvas, NumberInputModel* model) {
    const size_t text_x = 8;
    const size_t text_y = 25;

    elements_slightly_rounded_frame(canvas, 6, 14, 116, 15);

    canvas_draw_str(canvas, text_x, text_y, furi_string_get_cstr(model->text_buffer));
}

static bool number_input_use_sign(NumberInputModel* model) {
    //only show sign button if allowed number range needs it
    if(model->min_value < 0 && model->max_value >= 0) {
        return true;
    }
    return false;
}

static void number_input_backspace_cb(NumberInputModel* model) {
    size_t text_length = furi_string_utf8_length(model->text_buffer);
    if(text_length < 1 || (text_length < 2 && model->current_number <= 0)) {
        return;
    }
    furi_string_set_strn(
        model->text_buffer, furi_string_get_cstr(model->text_buffer), text_length - 1);
    model->current_number = strtol(furi_string_get_cstr(model->text_buffer), NULL, 10);
}

static void number_input_handle_up(NumberInputModel* model) {
    if(model->selected_row > 0) {
        model->selected_row--;
        if(model->selected_column > number_input_get_row_size(model->selected_row) - 1) {
            model->selected_column = number_input_get_row_size(model->selected_row) - 1;
        }
    }
}

static void number_input_handle_down(NumberInputModel* model) {
    if(model->selected_row < keyboard_row_count - 1) {
        if(model->selected_column >= number_input_get_row_size(model->selected_row) - 1) {
            model->selected_column = number_input_get_row_size(model->selected_row + 1) - 1;
        }
        model->selected_row += 1;
    }
    const NumberInputKey* keys = number_input_get_row(model->selected_row);
    if(keys[model->selected_column].text == sign_symbol && !number_input_use_sign(model)) {
        model->selected_column--;
    }
}

static void number_input_handle_left(NumberInputModel* model) {
    if(model->selected_column > 0) {
        model->selected_column--;
    } else {
        model->selected_column = number_input_get_row_size(model->selected_row) - 1;
    }
    const NumberInputKey* keys = number_input_get_row(model->selected_row);
    if(keys[model->selected_column].text == sign_symbol && !number_input_use_sign(model)) {
        model->selected_column--;
    }
}

static void number_input_handle_right(NumberInputModel* model) {
    if(model->selected_column < number_input_get_row_size(model->selected_row) - 1) {
        model->selected_column++;
    } else {
        model->selected_column = 0;
    }
    const NumberInputKey* keys = number_input_get_row(model->selected_row);
    if(keys[model->selected_column].text == sign_symbol && !number_input_use_sign(model)) {
        model->selected_column++;
    }
}

static bool is_number_too_large(NumberInputModel* model) {
    int64_t value;
    if(strint_to_int64(furi_string_get_cstr(model->text_buffer), NULL, &value, 10) !=
       StrintParseNoError) {
        return true;
    }
    if(value > (int64_t)model->max_value) {
        return true;
    }
    return false;
}

static bool is_number_too_small(NumberInputModel* model) {
    int64_t value;
    if(strint_to_int64(furi_string_get_cstr(model->text_buffer), NULL, &value, 10) !=
       StrintParseNoError) {
        return true;
    }
    if(value < (int64_t)model->min_value) {
        return true;
    }
    return false;
}

static void number_input_sign(NumberInputModel* model) {
    int32_t number = strtol(furi_string_get_cstr(model->text_buffer), NULL, 10);
    if(number == 0 && furi_string_cmp_str(model->text_buffer, "-") != 0) {
        furi_string_set_str(model->text_buffer, "-");
        return;
    }
    number = number * -1;
    furi_string_printf(model->text_buffer, "%ld", number);
    if(is_number_too_large(model) || is_number_too_small(model)) {
        furi_string_printf(model->text_buffer, "%ld", model->current_number);
        return;
    }
    model->current_number = strtol(furi_string_get_cstr(model->text_buffer), NULL, 10);
    if(model->current_number == 0) {
        furi_string_set_str(model->text_buffer, ""); //show empty if 0, better for usability
    }
}

static void number_input_add_digit(NumberInputModel* model, char* newChar) {
    furi_string_cat_str(model->text_buffer, newChar);
    if((model->max_value >= 0 && is_number_too_large(model)) ||
       (model->min_value < 0 && is_number_too_small(model))) {
        //you still need to be able to type invalid numbers in some cases to reach valid numbers on later keypress
        furi_string_printf(model->text_buffer, "%ld", model->current_number);
        return;
    }
    model->current_number = strtol(furi_string_get_cstr(model->text_buffer), NULL, 10);
    if(model->current_number == 0) {
        furi_string_reset(model->text_buffer);
    }
}

static void number_input_handle_ok(NumberInputModel* model) {
    char selected = number_input_get_row(model->selected_row)[model->selected_column].text;
    char temp_str[2] = {selected, '\0'};
    if(selected == enter_symbol) {
        if(is_number_too_large(model) || is_number_too_small(model)) {
            return; //Do nothing if number outside allowed range
        }
        model->current_number = strtol(furi_string_get_cstr(model->text_buffer), NULL, 10);
        model->callback(model->callback_context, model->current_number);
    } else if(selected == backspace_symbol) {
        number_input_backspace_cb(model);
    } else if(selected == sign_symbol) {
        number_input_sign(model);
    } else {
        number_input_add_digit(model, temp_str);
    }
}

static void number_input_view_draw_callback(Canvas* canvas, void* _model) {
    NumberInputModel* model = _model;

    number_input_draw_input(canvas, model);

    if(!furi_string_empty(model->header)) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 9, furi_string_get_cstr(model->header));
    }
    canvas_set_font(canvas, FontKeyboard);
    // Draw keyboard
    for(size_t row = 0; row < keyboard_row_count; row++) {
        const size_t column_count = number_input_get_row_size(row);
        const NumberInputKey* keys = number_input_get_row(row);

        for(size_t column = 0; column < column_count; column++) {
            if(keys[column].text == sign_symbol && !number_input_use_sign(model)) {
                continue;
            }

            if(keys[column].text == enter_symbol) {
                if(is_number_too_small(model) || is_number_too_large(model)) {
                    //in some cases you need to be able to type a number smaller/larger than the limits (expl. min = 50, clear all and editor must allow to type 9 and later 0 for 90)
                    if(model->selected_row == row && model->selected_column == column) {
                        canvas_draw_icon(
                            canvas,
                            keyboard_origin_x + keys[column].x,
                            keyboard_origin_y + keys[column].y,
                            &I_KeySaveBlockedSelected_24x11);
                    } else {
                        canvas_draw_icon(
                            canvas,
                            keyboard_origin_x + keys[column].x,
                            keyboard_origin_y + keys[column].y,
                            &I_KeySaveBlocked_24x11);
                    }
                } else {
                    if(model->selected_row == row && model->selected_column == column) {
                        canvas_draw_icon(
                            canvas,
                            keyboard_origin_x + keys[column].x,
                            keyboard_origin_y + keys[column].y,
                            &I_KeySaveSelected_24x11);
                    } else {
                        canvas_draw_icon(
                            canvas,
                            keyboard_origin_x + keys[column].x,
                            keyboard_origin_y + keys[column].y,
                            &I_KeySave_24x11);
                    }
                }
            } else if(keys[column].text == backspace_symbol) {
                if(model->selected_row == row && model->selected_column == column) {
                    canvas_draw_icon(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        &I_KeyBackspaceSelected_16x9);
                } else {
                    canvas_draw_icon(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        &I_KeyBackspace_16x9);
                }
            } else if(keys[column].text == sign_symbol) {
                if(model->selected_row == row && model->selected_column == column) {
                    canvas_draw_icon(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        &I_KeySignSelected_21x11);
                } else {
                    canvas_draw_icon(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        &I_KeySign_21x11);
                }
            } else {
                if(model->selected_row == row && model->selected_column == column) {
                    canvas_draw_box(
                        canvas,
                        keyboard_origin_x + keys[column].x - 3,
                        keyboard_origin_y + keys[column].y - 10,
                        11,
                        13);
                    canvas_set_color(canvas, ColorWhite);
                }

                canvas_draw_glyph(
                    canvas,
                    keyboard_origin_x + keys[column].x,
                    keyboard_origin_y + keys[column].y,
                    keys[column].text);
                canvas_set_color(canvas, ColorBlack);
            }
        }
    }
}

static bool number_input_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    NumberInput* number_input = context;

    bool consumed = false;

    // Fetch the model
    NumberInputModel* model = view_get_model(number_input->view);

    if(event->type == InputTypeShort || event->type == InputTypeLong ||
       event->type == InputTypeRepeat) {
        consumed = true;
        switch(event->key) {
        case InputKeyLeft:
            number_input_handle_left(model);
            break;
        case InputKeyRight:
            number_input_handle_right(model);
            break;
        case InputKeyUp:
            number_input_handle_up(model);
            break;
        case InputKeyDown:
            number_input_handle_down(model);
            break;
        case InputKeyOk:
            number_input_handle_ok(model);
            break;
        default:
            consumed = false;
            break;
        }
    }

    // commit view
    view_commit_model(number_input->view, consumed);

    return consumed;
}

NumberInput* number_input_alloc(void) {
    NumberInput* number_input = malloc(sizeof(NumberInput));
    number_input->view = view_alloc();
    view_set_context(number_input->view, number_input);
    view_allocate_model(number_input->view, ViewModelTypeLocking, sizeof(NumberInputModel));
    view_set_draw_callback(number_input->view, number_input_view_draw_callback);
    view_set_input_callback(number_input->view, number_input_view_input_callback);

    with_view_model(
        number_input->view,
        NumberInputModel * model,
        {
            model->header = furi_string_alloc();
            model->text_buffer = furi_string_alloc();
        },
        true);

    return number_input;
}

void number_input_free(NumberInput* number_input) {
    furi_check(number_input);
    with_view_model(
        number_input->view,
        NumberInputModel * model,
        {
            furi_string_free(model->header);
            furi_string_free(model->text_buffer);
        },
        true);
    view_free(number_input->view);
    free(number_input);
}

View* number_input_get_view(NumberInput* number_input) {
    furi_check(number_input);
    return number_input->view;
}

void number_input_set_result_callback(
    NumberInput* number_input,
    NumberInputCallback callback,
    void* callback_context,
    int32_t current_number,
    int32_t min_value,
    int32_t max_value) {
    furi_check(number_input);

    current_number = CLAMP(current_number, max_value, min_value);

    with_view_model(
        number_input->view,
        NumberInputModel * model,
        {
            model->callback = callback;
            model->callback_context = callback_context;
            model->current_number = current_number;
            furi_string_printf(model->text_buffer, "%ld", current_number);
            model->min_value = min_value;
            model->max_value = max_value;
        },
        true);
}

void number_input_set_header_text(NumberInput* number_input, const char* text) {
    furi_check(number_input);
    with_view_model(
        number_input->view,
        NumberInputModel * model,
        { furi_string_set(model->header, text); },
        true);
}
