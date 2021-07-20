#include "text_input.h"
#include <gui/elements.h>
#include <furi.h>

struct TextInput {
    View* view;
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

    TextInputCallback callback;
    void* callback_context;

    uint8_t selected_row;
    uint8_t selected_column;
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
        row_size = sizeof(keyboard_keys_row_1) / sizeof(TextInputKey);
        break;
    case 2:
        row_size = sizeof(keyboard_keys_row_2) / sizeof(TextInputKey);
        break;
    case 3:
        row_size = sizeof(keyboard_keys_row_3) / sizeof(TextInputKey);
        break;
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
    }

    return row;
}

static const char get_selected_char(TextInputModel* model) {
    return get_row(model->selected_row)[model->selected_column].text;
}

static const bool char_is_lowercase(char letter) {
    return (letter >= 0x61 && letter <= 0x7A);
}

static const char char_to_uppercase(const char letter) {
    return (letter - 0x20);
}

static void text_input_backspace_cb(TextInputModel* model) {
    uint8_t text_length = strlen(model->text_buffer);
    if(text_length > 0) {
        model->text_buffer[text_length - 1] = 0;
    }
}

static void text_input_view_draw_callback(Canvas* canvas, void* _model) {
    TextInputModel* model = _model;
    uint8_t text_length = strlen(model->text_buffer);
    uint8_t needed_string_width = canvas_width(canvas) - 8;
    uint8_t start_pos = 4;

    const char* text = model->text_buffer;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_str(canvas, 2, 8, model->header);
    elements_slightly_rounded_frame(canvas, 1, 12, 126, 15);

    if(canvas_string_width(canvas, text) > needed_string_width) {
        canvas_draw_str(canvas, start_pos, 22, "...");
        start_pos += 6;
        needed_string_width -= 8;
    }

    while(text != 0 && canvas_string_width(canvas, text) > needed_string_width) {
        text++;
    }

    canvas_draw_str(canvas, start_pos, 22, text);

    canvas_draw_str(canvas, start_pos + canvas_string_width(canvas, text) + 1, 22, "|");
    canvas_draw_str(canvas, start_pos + canvas_string_width(canvas, text) + 2, 22, "|");

    canvas_set_font(canvas, FontKeyboard);

    for(uint8_t row = 0; row <= keyboard_row_count; row++) {
        const uint8_t column_count = get_row_size(row);
        const TextInputKey* keys = get_row(row);

        for(size_t column = 0; column < column_count; column++) {
            if(keys[column].text == ENTER_KEY) {
                canvas_set_color(canvas, ColorBlack);
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
            } else if(keys[column].text == BACKSPACE_KEY) {
                canvas_set_color(canvas, ColorBlack);
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
            } else {
                if(model->selected_row == row && model->selected_column == column) {
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

                if(text_length == 0 && char_is_lowercase(keys[column].text)) {
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
}

static void text_input_handle_up(TextInput* text_input) {
    with_view_model(
        text_input->view, (TextInputModel * model) {
            if(model->selected_row > 0) {
                model->selected_row--;
                if(model->selected_column > get_row_size(model->selected_row) - 6) {
                    model->selected_column = model->selected_column + 1;
                }
            }
            return true;
        });
}

static void text_input_handle_down(TextInput* text_input) {
    with_view_model(
        text_input->view, (TextInputModel * model) {
            if(model->selected_row < keyboard_row_count - 1) {
                model->selected_row++;
                if(model->selected_column > get_row_size(model->selected_row) - 4) {
                    model->selected_column = model->selected_column - 1;
                }
            }
            return true;
        });
}

static void text_input_handle_left(TextInput* text_input) {
    with_view_model(
        text_input->view, (TextInputModel * model) {
            if(model->selected_column > 0) {
                model->selected_column--;
            } else {
                model->selected_column = get_row_size(model->selected_row) - 1;
            }
            return true;
        });
}

static void text_input_handle_right(TextInput* text_input) {
    with_view_model(
        text_input->view, (TextInputModel * model) {
            if(model->selected_column < get_row_size(model->selected_row) - 1) {
                model->selected_column++;
            } else {
                model->selected_column = 0;
            }
            return true;
        });
}

static void text_input_handle_ok(TextInput* text_input) {
    with_view_model(
        text_input->view, (TextInputModel * model) {
            char selected = get_selected_char(model);
            uint8_t text_length = strlen(model->text_buffer);

            if(selected == ENTER_KEY) {
                if(model->callback != 0 && text_length > 0) {
                    model->callback(model->callback_context);
                }
            } else if(selected == BACKSPACE_KEY) {
                text_input_backspace_cb(model);
            } else if(text_length < (model->text_buffer_size - 1)) {
                if(text_length == 0 && char_is_lowercase(selected)) {
                    selected = char_to_uppercase(selected);
                }
                model->text_buffer[text_length] = selected;
                model->text_buffer[text_length + 1] = 0;
            }
            return true;
        });
}

static bool text_input_view_input_callback(InputEvent* event, void* context) {
    TextInput* text_input = context;
    furi_assert(text_input);
    bool consumed = false;

    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
        case InputKeyUp:
            text_input_handle_up(text_input);
            consumed = true;
            break;
        case InputKeyDown:
            text_input_handle_down(text_input);
            consumed = true;
            break;
        case InputKeyLeft:
            text_input_handle_left(text_input);
            consumed = true;
            break;
        case InputKeyRight:
            text_input_handle_right(text_input);
            consumed = true;
            break;
        case InputKeyOk:
            text_input_handle_ok(text_input);
            consumed = true;
            break;
        default:
            break;
        }
    }

    if((event->type == InputTypeLong || event->type == InputTypeRepeat) &&
       event->key == InputKeyBack) {
        with_view_model(
            text_input->view, (TextInputModel * model) {
                text_input_backspace_cb(model);
                return true;
            });

        consumed = true;
    }

    return consumed;
}

TextInput* text_input_alloc() {
    TextInput* text_input = furi_alloc(sizeof(TextInput));
    text_input->view = view_alloc();
    view_set_context(text_input->view, text_input);
    view_allocate_model(text_input->view, ViewModelTypeLocking, sizeof(TextInputModel));
    view_set_draw_callback(text_input->view, text_input_view_draw_callback);
    view_set_input_callback(text_input->view, text_input_view_input_callback);

    with_view_model(
        text_input->view, (TextInputModel * model) {
            model->text_buffer_size = 0;
            model->header = "";
            model->selected_row = 0;
            model->selected_column = 0;
            return true;
        });

    return text_input;
}

void text_input_free(TextInput* text_input) {
    furi_assert(text_input);
    view_free(text_input->view);
    free(text_input);
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
    size_t text_buffer_size) {
    with_view_model(
        text_input->view, (TextInputModel * model) {
            model->callback = callback;
            model->callback_context = callback_context;
            model->text_buffer = text_buffer;
            model->text_buffer_size = text_buffer_size;
            return true;
        });
}

void text_input_set_header_text(TextInput* text_input, const char* text) {
    with_view_model(
        text_input->view, (TextInputModel * model) {
            model->header = text;
            return true;
        });
}