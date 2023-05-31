#include "text_input.h"
#include <gui/elements.h>
#include <assets_icons.h>
#include <furi.h>

struct TextInput {
    View* view;
    FuriTimer* timer;
};

typedef struct {
    const char text;
    const uint8_t x;
    const uint8_t y;
} TextInputKey;

typedef struct {
    const char* header;
    char* text_buffer;
    size_t text_buffer_size;
    size_t minimum_length;
    bool clear_default_text;

    bool cursor_select;
    size_t cursor_pos;

    TextInputCallback callback;
    void* callback_context;

    uint8_t selected_row;
    uint8_t selected_column;

    TextInputValidatorCallback validator_callback;
    void* validator_callback_context;
    FuriString* validator_text;
    bool validator_message_visible;
} TextInputModel;

static const uint8_t keyboard_origin_x = 1;
static const uint8_t keyboard_origin_y = 29;
static const uint8_t keyboard_row_count = 3;

#define ENTER_KEY '\r'
#define BACKSPACE_KEY '\b'

static const TextInputKey keyboard_keys_row_1[] = {
    {'q', 1, 8},
    {'w', 10, 8},
    {'e', 19, 8},
    {'r', 28, 8},
    {'t', 37, 8},
    {'y', 46, 8},
    {'u', 55, 8},
    {'i', 64, 8},
    {'o', 73, 8},
    {'p', 82, 8},
    {'0', 91, 8},
    {'1', 100, 8},
    {'2', 110, 8},
    {'3', 120, 8},
};

static const TextInputKey keyboard_keys_row_2[] = {
    {'a', 1, 20},
    {'s', 10, 20},
    {'d', 19, 20},
    {'f', 28, 20},
    {'g', 37, 20},
    {'h', 46, 20},
    {'j', 55, 20},
    {'k', 64, 20},
    {'l', 73, 20},
    {BACKSPACE_KEY, 82, 12},
    {'4', 100, 20},
    {'5', 110, 20},
    {'6', 120, 20},
};

static const TextInputKey keyboard_keys_row_3[] = {
    {'z', 1, 32},
    {'x', 10, 32},
    {'c', 19, 32},
    {'v', 28, 32},
    {'b', 37, 32},
    {'n', 46, 32},
    {'m', 55, 32},
    {'_', 64, 32},
    {ENTER_KEY, 74, 23},
    {'7', 100, 32},
    {'8', 110, 32},
    {'9', 120, 32},
};

static uint8_t get_row_size(uint8_t row_index) {
    uint8_t row_size = 0;

    switch(row_index + 1) {
    case 1:
        row_size = COUNT_OF(keyboard_keys_row_1);
        break;
    case 2:
        row_size = COUNT_OF(keyboard_keys_row_2);
        break;
    case 3:
        row_size = COUNT_OF(keyboard_keys_row_3);
        break;
    default:
        furi_crash(NULL);
    }

    return row_size;
}

static const TextInputKey* get_row(uint8_t row_index) {
    const TextInputKey* row = NULL;

    switch(row_index + 1) {
    case 1:
        row = keyboard_keys_row_1;
        break;
    case 2:
        row = keyboard_keys_row_2;
        break;
    case 3:
        row = keyboard_keys_row_3;
        break;
    default:
        furi_crash(NULL);
    }

    return row;
}

static char get_selected_char(TextInputModel* model) {
    return get_row(model->selected_row)[model->selected_column].text;
}

static bool char_is_lowercase(char letter) {
    return (letter >= 0x61 && letter <= 0x7A);
}

static char char_to_uppercase(const char letter) {
    if(letter == '_') {
        return 0x20;
    } else if(char_is_lowercase(letter)) {
        return (letter - 0x20);
    } else {
        return letter;
    }
}

static void text_input_backspace_cb(TextInputModel* model) {
    if(model->clear_default_text) {
        model->text_buffer[0] = 0;
        model->cursor_pos = 0;
    } else if(model->cursor_pos > 0) {
        char* move = model->text_buffer + model->cursor_pos;
        memmove(move - 1, move, strlen(move) + 1);
        model->cursor_pos--;
    }
}

static void text_input_view_draw_callback(Canvas* canvas, void* _model) {
    TextInputModel* model = _model;
    uint8_t text_length = model->text_buffer ? strlen(model->text_buffer) : 0;
    uint8_t needed_string_width = canvas_width(canvas) - 8;
    uint8_t start_pos = 4;

    model->cursor_pos = model->cursor_pos > text_length ? text_length : model->cursor_pos;
    size_t cursor_pos = model->cursor_pos;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_str(canvas, 2, 8, model->header);
    elements_slightly_rounded_frame(canvas, 1, 12, 126, 15);

    char buf[text_length + 1];
    if(model->text_buffer) {
        strlcpy(buf, model->text_buffer, sizeof(buf));
    }
    char* str = buf;

    if(model->clear_default_text) {
        elements_slightly_rounded_box(
            canvas, start_pos - 1, 14, canvas_string_width(canvas, str) + 2, 10);
        canvas_set_color(canvas, ColorWhite);
    } else {
        char* move = str + cursor_pos;
        memmove(move + 1, move, strlen(move) + 1);
        str[cursor_pos] = '|';
    }

    if(cursor_pos > 0 && canvas_string_width(canvas, str) > needed_string_width) {
        canvas_draw_str(canvas, start_pos, 22, "...");
        start_pos += 6;
        needed_string_width -= 8;
        for(uint32_t off = 0;
            strlen(str) && canvas_string_width(canvas, str) > needed_string_width &&
            off < cursor_pos;
            off++) {
            str++;
        }
    }

    if(canvas_string_width(canvas, str) > needed_string_width) {
        needed_string_width -= 4;
        size_t len = strlen(str);
        while(len && canvas_string_width(canvas, str) > needed_string_width) {
            str[len--] = '\0';
        }
        strcat(str, "...");
    }

    canvas_draw_str(canvas, start_pos, 22, str);

    canvas_set_font(canvas, FontKeyboard);

    for(uint8_t row = 0; row < keyboard_row_count; row++) {
        const uint8_t column_count = get_row_size(row);
        const TextInputKey* keys = get_row(row);

        for(size_t column = 0; column < column_count; column++) {
            bool selected = !model->cursor_select && model->selected_row == row &&
                            model->selected_column == column;

            if(keys[column].text == ENTER_KEY) {
                canvas_set_color(canvas, ColorBlack);
                if(selected) {
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
            } else if(keys[column].text == BACKSPACE_KEY) {
                canvas_set_color(canvas, ColorBlack);
                if(selected) {
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
            } else {
                if(selected) {
                    canvas_set_color(canvas, ColorBlack);
                    canvas_draw_box(
                        canvas,
                        keyboard_origin_x + keys[column].x - 1,
                        keyboard_origin_y + keys[column].y - 8,
                        7,
                        10);
                    canvas_set_color(canvas, ColorWhite);
                } else {
                    canvas_set_color(canvas, ColorBlack);
                }

                if(model->clear_default_text || text_length == 0) {
                    canvas_draw_glyph(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        char_to_uppercase(keys[column].text));
                } else {
                    canvas_draw_glyph(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        keys[column].text);
                }
            }
        }
    }
    if(model->validator_message_visible) {
        canvas_set_font(canvas, FontSecondary);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 8, 10, 110, 48);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_icon(canvas, 10, 14, &I_WarningDolphin_45x42);
        canvas_draw_rframe(canvas, 8, 8, 112, 50, 3);
        canvas_draw_rframe(canvas, 9, 9, 110, 48, 2);
        elements_multiline_text(canvas, 62, 20, furi_string_get_cstr(model->validator_text));
        canvas_set_font(canvas, FontKeyboard);
    }
}

static void text_input_handle_up(TextInput* text_input, TextInputModel* model) {
    UNUSED(text_input);
    if(model->selected_row > 0) {
        model->selected_row--;
        if(model->selected_column > get_row_size(model->selected_row) - 6) {
            model->selected_column = model->selected_column + 1;
        }
    } else {
        model->cursor_select = true;
        model->clear_default_text = false;
    }
}

static void text_input_handle_down(TextInput* text_input, TextInputModel* model) {
    UNUSED(text_input);
    if(model->cursor_select) {
        model->cursor_select = false;
    } else if(model->selected_row < keyboard_row_count - 1) {
        model->selected_row++;
        if(model->selected_column > get_row_size(model->selected_row) - 4) {
            model->selected_column = model->selected_column - 1;
        }
    }
}

static void text_input_handle_left(TextInput* text_input, TextInputModel* model) {
    UNUSED(text_input);
    if(model->cursor_select) {
        if(model->cursor_pos > 0) {
            model->cursor_pos = CLAMP(model->cursor_pos - 1, strlen(model->text_buffer), 0u);
        }
    } else if(model->selected_column > 0) {
        model->selected_column--;
    } else {
        model->selected_column = get_row_size(model->selected_row) - 1;
    }
}

static void text_input_handle_right(TextInput* text_input, TextInputModel* model) {
    UNUSED(text_input);
    if(model->cursor_select) {
        model->cursor_pos = CLAMP(model->cursor_pos + 1, strlen(model->text_buffer), 0u);
    } else if(model->selected_column < get_row_size(model->selected_row) - 1) {
        model->selected_column++;
    } else {
        model->selected_column = 0;
    }
}

static void text_input_handle_ok(TextInput* text_input, TextInputModel* model, InputType type) {
    if(model->cursor_select) return;
    bool shift = type == InputTypeLong;
    bool repeat = type == InputTypeRepeat;
    char selected = get_selected_char(model);
    size_t text_length = strlen(model->text_buffer);

    if(selected == ENTER_KEY) {
        if(model->validator_callback &&
           (!model->validator_callback(
               model->text_buffer, model->validator_text, model->validator_callback_context))) {
            model->validator_message_visible = true;
            furi_timer_start(text_input->timer, furi_kernel_get_tick_frequency() * 4);
        } else if(model->callback != 0 && text_length >= model->minimum_length) {
            model->callback(model->callback_context);
        }
    } else {
        if(selected == BACKSPACE_KEY) {
            text_input_backspace_cb(model);
        } else if(!repeat) {
            if(model->clear_default_text) {
                text_length = 0;
            }
            if(text_length < (model->text_buffer_size - 1)) {
                if(shift != (text_length == 0)) {
                    selected = char_to_uppercase(selected);
                }
                if(model->clear_default_text) {
                    model->text_buffer[0] = selected;
                    model->text_buffer[1] = '\0';
                    model->cursor_pos = 1;
                } else {
                    char* move = model->text_buffer + model->cursor_pos;
                    memmove(move + 1, move, strlen(move) + 1);
                    model->text_buffer[model->cursor_pos] = selected;
                    model->cursor_pos++;
                }
            }
        }
        model->clear_default_text = false;
    }
}

static bool text_input_view_input_callback(InputEvent* event, void* context) {
    TextInput* text_input = context;
    furi_assert(text_input);

    bool consumed = false;

    // Acquire model
    TextInputModel* model = view_get_model(text_input->view);

    if((!(event->type == InputTypePress) && !(event->type == InputTypeRelease)) &&
       model->validator_message_visible) {
        model->validator_message_visible = false;
        consumed = true;
    } else if(event->type == InputTypeShort) {
        consumed = true;
        switch(event->key) {
        case InputKeyUp:
            text_input_handle_up(text_input, model);
            break;
        case InputKeyDown:
            text_input_handle_down(text_input, model);
            break;
        case InputKeyLeft:
            text_input_handle_left(text_input, model);
            break;
        case InputKeyRight:
            text_input_handle_right(text_input, model);
            break;
        case InputKeyOk:
            text_input_handle_ok(text_input, model, event->type);
            break;
        default:
            consumed = false;
            break;
        }
    } else if(event->type == InputTypeLong) {
        consumed = true;
        switch(event->key) {
        case InputKeyUp:
            text_input_handle_up(text_input, model);
            break;
        case InputKeyDown:
            text_input_handle_down(text_input, model);
            break;
        case InputKeyLeft:
            text_input_handle_left(text_input, model);
            break;
        case InputKeyRight:
            text_input_handle_right(text_input, model);
            break;
        case InputKeyOk:
            text_input_handle_ok(text_input, model, event->type);
            break;
        case InputKeyBack:
            text_input_backspace_cb(model);
            break;
        default:
            consumed = false;
            break;
        }
    } else if(event->type == InputTypeRepeat) {
        consumed = true;
        switch(event->key) {
        case InputKeyUp:
            text_input_handle_up(text_input, model);
            break;
        case InputKeyDown:
            text_input_handle_down(text_input, model);
            break;
        case InputKeyLeft:
            text_input_handle_left(text_input, model);
            break;
        case InputKeyRight:
            text_input_handle_right(text_input, model);
            break;
        case InputKeyOk:
            text_input_handle_ok(text_input, model, event->type);
            break;
        case InputKeyBack:
            text_input_backspace_cb(model);
            break;
        default:
            consumed = false;
            break;
        }
    }

    // Commit model
    view_commit_model(text_input->view, consumed);

    return consumed;
}

void text_input_timer_callback(void* context) {
    furi_assert(context);
    TextInput* text_input = context;

    with_view_model(
        text_input->view,
        TextInputModel * model,
        { model->validator_message_visible = false; },
        true);
}

TextInput* text_input_alloc() {
    TextInput* text_input = malloc(sizeof(TextInput));
    text_input->view = view_alloc();
    view_set_context(text_input->view, text_input);
    view_allocate_model(text_input->view, ViewModelTypeLocking, sizeof(TextInputModel));
    view_set_draw_callback(text_input->view, text_input_view_draw_callback);
    view_set_input_callback(text_input->view, text_input_view_input_callback);

    text_input->timer = furi_timer_alloc(text_input_timer_callback, FuriTimerTypeOnce, text_input);

    with_view_model(
        text_input->view,
        TextInputModel * model,
        {
            model->validator_text = furi_string_alloc();
            model->minimum_length = 1;
            model->cursor_pos = 0;
            model->cursor_select = false;
        },
        false);

    text_input_reset(text_input);

    return text_input;
}

void text_input_free(TextInput* text_input) {
    furi_assert(text_input);
    with_view_model(
        text_input->view,
        TextInputModel * model,
        { furi_string_free(model->validator_text); },
        false);

    // Send stop command
    furi_timer_stop(text_input->timer);
    // Release allocated memory
    furi_timer_free(text_input->timer);

    view_free(text_input->view);

    free(text_input);
}

void text_input_reset(TextInput* text_input) {
    furi_assert(text_input);
    with_view_model(
        text_input->view,
        TextInputModel * model,
        {
            model->header = "";
            model->selected_row = 0;
            model->selected_column = 0;
            model->minimum_length = 1;
            model->clear_default_text = false;
            model->cursor_pos = 0;
            model->cursor_select = false;
            model->text_buffer = NULL;
            model->text_buffer_size = 0;
            model->callback = NULL;
            model->callback_context = NULL;
            model->validator_callback = NULL;
            model->validator_callback_context = NULL;
            furi_string_reset(model->validator_text);
            model->validator_message_visible = false;
        },
        true);
}

View* text_input_get_view(TextInput* text_input) {
    furi_assert(text_input);
    return text_input->view;
}

void text_input_set_result_callback(
    TextInput* text_input,
    TextInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text) {
    with_view_model(
        text_input->view,
        TextInputModel * model,
        {
            model->callback = callback;
            model->callback_context = callback_context;
            model->text_buffer = text_buffer;
            model->text_buffer_size = text_buffer_size;
            model->clear_default_text = clear_default_text;
            model->cursor_select = false;
            if(text_buffer && text_buffer[0] != '\0') {
                model->cursor_pos = strlen(text_buffer);
                // Set focus on Save
                model->selected_row = 2;
                model->selected_column = 8;
            } else {
                model->cursor_pos = 0;
            }
        },
        true);
}

void text_input_set_minimum_length(TextInput* text_input, size_t minimum_length) {
    with_view_model(
        text_input->view,
        TextInputModel * model,
        { model->minimum_length = minimum_length; },
        true);
}

void text_input_set_validator(
    TextInput* text_input,
    TextInputValidatorCallback callback,
    void* callback_context) {
    with_view_model(
        text_input->view,
        TextInputModel * model,
        {
            model->validator_callback = callback;
            model->validator_callback_context = callback_context;
        },
        true);
}

TextInputValidatorCallback text_input_get_validator_callback(TextInput* text_input) {
    TextInputValidatorCallback validator_callback = NULL;
    with_view_model(
        text_input->view,
        TextInputModel * model,
        { validator_callback = model->validator_callback; },
        false);
    return validator_callback;
}

void* text_input_get_validator_callback_context(TextInput* text_input) {
    void* validator_callback_context = NULL;
    with_view_model(
        text_input->view,
        TextInputModel * model,
        { validator_callback_context = model->validator_callback_context; },
        false);
    return validator_callback_context;
}

void text_input_set_header_text(TextInput* text_input, const char* text) {
    with_view_model(
        text_input->view, TextInputModel * model, { model->header = text; }, true);
}
