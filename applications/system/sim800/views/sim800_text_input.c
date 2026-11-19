#include "sim800_text_input.h"

#include <gui/elements.h>
#include <furi.h>

#include <sim800_icons.h>

struct Sim800TextInput {
    View* view;
};

typedef struct {
    const char text;
    const uint8_t x;
    const uint8_t y;
} Sim800TextInputKey;

typedef struct {
    const Sim800TextInputKey* rows[3];
    const uint8_t keyboard_index;
} Keyboard;

typedef struct {
    const char* header;
    char* text_buffer;
    size_t text_buffer_size;
    size_t minimum_length;
    bool clear_default_text;

    bool cursor_select;
    size_t cursor_pos;

    Sim800TextInputCallback callback;
    void* callback_context;

    uint8_t selected_row;
    uint8_t selected_column;
    uint8_t selected_keyboard;
} Sim800TextInputModel;

static const uint8_t keyboard_origin_x = 1;
static const uint8_t keyboard_origin_y = 29;
static const uint8_t keyboard_row_count = 3;
static const uint8_t keyboard_count = 2;

#define ENTER_KEY           '\r'
#define BACKSPACE_KEY       '\b'
#define SWITCH_KEYBOARD_KEY 0xfe

static const Sim800TextInputKey keyboard_keys_row_1[] = {
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

static const Sim800TextInputKey keyboard_keys_row_2[] = {
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

static const Sim800TextInputKey keyboard_keys_row_3[] = {
    {SWITCH_KEYBOARD_KEY, 1, 23},
    {'z', 13, 32},
    {'x', 21, 32},
    {'c', 28, 32},
    {'v', 36, 32},
    {'b', 44, 32},
    {'n', 52, 32},
    {'m', 59, 32},
    {'_', 67, 32},
    {ENTER_KEY, 74, 23},
    {'7', 100, 32},
    {'8', 110, 32},
    {'9', 120, 32},
};

// "+" for numbers, "*" and "#" for USSD, quote and "=" for hand-typed AT commands
static const Sim800TextInputKey symbol_keyboard_keys_row_1[] = {
    {'+', 2, 8},
    {'*', 12, 8},
    {'#', 22, 8},
    {'@', 32, 8},
    {'!', 42, 8},
    {'?', 52, 8},
    {'&', 62, 8},
    {'(', 71, 8},
    {')', 81, 8},
    {'0', 91, 8},
    {'1', 100, 8},
    {'2', 110, 8},
    {'3', 120, 8},
};

static const Sim800TextInputKey symbol_keyboard_keys_row_2[] = {
    {'"', 2, 20},
    {'\'', 12, 20},
    {',', 22, 20},
    {'.', 32, 20},
    {':', 42, 20},
    {';', 52, 20},
    {'=', 62, 20},
    {'-', 72, 20},
    {BACKSPACE_KEY, 82, 12},
    {'4', 100, 20},
    {'5', 110, 20},
    {'6', 120, 20},
};

static const Sim800TextInputKey symbol_keyboard_keys_row_3[] = {
    {SWITCH_KEYBOARD_KEY, 1, 23},
    {'/', 15, 32},
    {'\\', 29, 32},
    {'$', 41, 32},
    {'%', 53, 32},
    {'~', 65, 32},
    {ENTER_KEY, 74, 23},
    {'7', 100, 32},
    {'8', 110, 32},
    {'9', 120, 32},
};

static const Keyboard keyboard = {
    .rows =
        {
            keyboard_keys_row_1,
            keyboard_keys_row_2,
            keyboard_keys_row_3,
        },
    .keyboard_index = 0,
};

static const Keyboard symbol_keyboard = {
    .rows =
        {
            symbol_keyboard_keys_row_1,
            symbol_keyboard_keys_row_2,
            symbol_keyboard_keys_row_3,
        },
    .keyboard_index = 1,
};

static const Keyboard* keyboards[] = {
    &keyboard,
    &symbol_keyboard,
};

static void switch_keyboard(Sim800TextInputModel* model) {
    model->selected_keyboard = (model->selected_keyboard + 1) % keyboard_count;
}

static uint8_t get_row_size(const Keyboard* keyboard_instance, uint8_t row_index) {
    uint8_t row_size = 0;

    if(keyboard_instance == &symbol_keyboard) {
        switch(row_index) {
        case 0:
            row_size = COUNT_OF(symbol_keyboard_keys_row_1);
            break;
        case 1:
            row_size = COUNT_OF(symbol_keyboard_keys_row_2);
            break;
        case 2:
            row_size = COUNT_OF(symbol_keyboard_keys_row_3);
            break;
        default:
            furi_crash(NULL);
        }
    } else {
        switch(row_index) {
        case 0:
            row_size = COUNT_OF(keyboard_keys_row_1);
            break;
        case 1:
            row_size = COUNT_OF(keyboard_keys_row_2);
            break;
        case 2:
            row_size = COUNT_OF(keyboard_keys_row_3);
            break;
        default:
            furi_crash(NULL);
        }
    }

    return row_size;
}

static const Sim800TextInputKey* get_row(const Keyboard* keyboard_instance, uint8_t row_index) {
    furi_check(row_index < 3);
    return keyboard_instance->rows[row_index];
}

static char get_selected_char(Sim800TextInputModel* model) {
    return get_row(
               keyboards[model->selected_keyboard], model->selected_row)[model->selected_column]
        .text;
}

static bool char_is_lowercase(char letter) {
    return (letter >= 0x61 && letter <= 0x7A);
}

static char char_to_uppercase(const char letter) {
    if(letter == '_') {
        return 0x20; // the only way to type a space
    } else if(char_is_lowercase(letter)) {
        return (letter - 0x20);
    } else {
        return letter;
    }
}

static void sim800_text_input_backspace_cb(Sim800TextInputModel* model) {
    if(model->clear_default_text) {
        model->text_buffer[0] = 0;
        model->cursor_pos = 0;
    } else if(model->cursor_pos > 0) {
        char* move = model->text_buffer + model->cursor_pos;
        memmove(move - 1, move, strlen(move) + 1);
        model->cursor_pos--;
    }
}

static void sim800_text_input_view_draw_callback(Canvas* canvas, void* _model) {
    Sim800TextInputModel* model = _model;
    uint8_t text_length = model->text_buffer ? strlen(model->text_buffer) : 0;
    uint8_t needed_string_width = canvas_width(canvas) - 8;
    uint8_t start_pos = 4;

    model->cursor_pos = model->cursor_pos > text_length ? text_length : model->cursor_pos;
    size_t cursor_pos = model->cursor_pos;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_str(canvas, 2, 8, model->header);
    elements_slightly_rounded_frame(canvas, 1, 12, 126, 15);

    char buf[model->text_buffer_size + 1];
    if(model->text_buffer) {
        strlcpy(buf, model->text_buffer, sizeof(buf));
    } else {
        buf[0] = '\0';
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
            str[--len] = '\0';
        }
        // Mark the cut in place: buf has no room left to append an ellipsis
        if(len >= 3) {
            str[len - 3] = '.';
            str[len - 2] = '.';
            str[len - 1] = '.';
        }
    }

    canvas_draw_str(canvas, start_pos, 22, str);

    canvas_set_font(canvas, FontKeyboard);

    for(uint8_t row = 0; row < keyboard_row_count; row++) {
        const uint8_t column_count = get_row_size(keyboards[model->selected_keyboard], row);
        const Sim800TextInputKey* keys = get_row(keyboards[model->selected_keyboard], row);

        for(size_t column = 0; column < column_count; column++) {
            bool selected = !model->cursor_select && model->selected_row == row &&
                            model->selected_column == column;
            const Icon* icon = NULL;
            if(keys[column].text == ENTER_KEY) {
                icon = selected ? &I_KeySaveSelected_24x11 : &I_KeySave_24x11;
            } else if(keys[column].text == SWITCH_KEYBOARD_KEY) {
                icon = selected ? &I_KeyKeyboardSelected_10x11 : &I_KeyKeyboard_10x11;
            } else if(keys[column].text == BACKSPACE_KEY) {
                icon = selected ? &I_KeyBackspaceSelected_16x9 : &I_KeyBackspace_16x9;
            }

            canvas_set_color(canvas, ColorBlack);
            if(icon != NULL) {
                canvas_draw_icon(
                    canvas,
                    keyboard_origin_x + keys[column].x,
                    keyboard_origin_y + keys[column].y,
                    icon);
            } else {
                if(selected) {
                    canvas_draw_box(
                        canvas,
                        keyboard_origin_x + keys[column].x - 1,
                        keyboard_origin_y + keys[column].y - 8,
                        7,
                        10);
                    canvas_set_color(canvas, ColorWhite);
                }

                canvas_draw_glyph(
                    canvas,
                    keyboard_origin_x + keys[column].x,
                    keyboard_origin_y + keys[column].y,
                    (model->clear_default_text || text_length == 0) ?
                        char_to_uppercase(keys[column].text) :
                        keys[column].text);
            }
        }
    }
}

static void sim800_text_input_handle_up(Sim800TextInputModel* model) {
    if(model->selected_row > 0) {
        model->selected_row--;
        if(model->selected_row == 0 &&
           model->selected_column >
               get_row_size(keyboards[model->selected_keyboard], model->selected_row) - 6) {
            model->selected_column = model->selected_column + 1;
        }
        if(model->selected_row == 1 &&
           model->selected_keyboard == symbol_keyboard.keyboard_index) {
            if(model->selected_column > 5)
                model->selected_column += 2;
            else if(model->selected_column > 1)
                model->selected_column += 1;
        }
    } else {
        model->cursor_select = true;
        model->clear_default_text = false;
    }
}

static void sim800_text_input_handle_down(Sim800TextInputModel* model) {
    if(model->cursor_select) {
        model->cursor_select = false;
    } else if(model->selected_row < keyboard_row_count - 1) {
        model->selected_row++;
        if(model->selected_row == 1 &&
           model->selected_column >
               get_row_size(keyboards[model->selected_keyboard], model->selected_row) - 4) {
            model->selected_column = model->selected_column - 1;
        }
        if(model->selected_row == 2 &&
           model->selected_keyboard == symbol_keyboard.keyboard_index) {
            if(model->selected_column > 7)
                model->selected_column -= 2;
            else if(model->selected_column > 1)
                model->selected_column -= 1;
        }
    }
}

static void sim800_text_input_handle_left(Sim800TextInputModel* model) {
    if(model->cursor_select) {
        if(model->cursor_pos > 0) {
            model->cursor_pos = CLAMP(model->cursor_pos - 1, strlen(model->text_buffer), 0u);
        }
    } else if(model->selected_column > 0) {
        model->selected_column--;
    } else {
        model->selected_column =
            get_row_size(keyboards[model->selected_keyboard], model->selected_row) - 1;
    }
}

static void sim800_text_input_handle_right(Sim800TextInputModel* model) {
    if(model->cursor_select) {
        model->cursor_pos = CLAMP(model->cursor_pos + 1, strlen(model->text_buffer), 0u);
    } else if(
        model->selected_column <
        get_row_size(keyboards[model->selected_keyboard], model->selected_row) - 1) {
        model->selected_column++;
    } else {
        model->selected_column = 0;
    }
}

static void sim800_text_input_handle_ok(Sim800TextInputModel* model, InputType type) {
    if(model->cursor_select) return;

    bool shift = type == InputTypeLong;
    bool repeat = type == InputTypeRepeat;
    char selected = get_selected_char(model);
    size_t text_length = strlen(model->text_buffer);

    if(selected == ENTER_KEY) {
        if(model->callback != NULL && text_length >= model->minimum_length) {
            model->callback(model->callback_context);
        }
    } else if(selected == SWITCH_KEYBOARD_KEY) {
        switch_keyboard(model);
    } else {
        if(selected == BACKSPACE_KEY) {
            sim800_text_input_backspace_cb(model);
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

static bool sim800_text_input_view_input_callback(InputEvent* event, void* context) {
    Sim800TextInput* text_input = context;
    furi_assert(text_input);

    if(event->type != InputTypeShort && event->type != InputTypeLong &&
       event->type != InputTypeRepeat) {
        return false;
    }

    bool consumed = true;
    Sim800TextInputModel* model = view_get_model(text_input->view);

    switch(event->key) {
    case InputKeyUp:
        sim800_text_input_handle_up(model);
        break;
    case InputKeyDown:
        sim800_text_input_handle_down(model);
        break;
    case InputKeyLeft:
        sim800_text_input_handle_left(model);
        break;
    case InputKeyRight:
        sim800_text_input_handle_right(model);
        break;
    case InputKeyOk:
        sim800_text_input_handle_ok(model, event->type);
        break;
    case InputKeyBack:
        // Short back leaves the screen; held, it deletes
        if(event->type == InputTypeShort) {
            consumed = false;
        } else {
            sim800_text_input_backspace_cb(model);
        }
        break;
    default:
        consumed = false;
        break;
    }

    view_commit_model(text_input->view, consumed);

    return consumed;
}

Sim800TextInput* sim800_text_input_alloc(void) {
    Sim800TextInput* text_input = malloc(sizeof(Sim800TextInput));

    text_input->view = view_alloc();
    view_set_context(text_input->view, text_input);
    view_allocate_model(text_input->view, ViewModelTypeLocking, sizeof(Sim800TextInputModel));
    view_set_draw_callback(text_input->view, sim800_text_input_view_draw_callback);
    view_set_input_callback(text_input->view, sim800_text_input_view_input_callback);

    sim800_text_input_reset(text_input);

    return text_input;
}

void sim800_text_input_free(Sim800TextInput* text_input) {
    furi_assert(text_input);
    view_free(text_input->view);
    free(text_input);
}

void sim800_text_input_reset(Sim800TextInput* text_input) {
    furi_assert(text_input);
    with_view_model(
        text_input->view,
        Sim800TextInputModel * model,
        {
            model->header = "";
            model->selected_row = 0;
            model->selected_column = 0;
            model->selected_keyboard = 0;
            model->minimum_length = 1;
            model->clear_default_text = false;
            model->cursor_pos = 0;
            model->cursor_select = false;
            model->text_buffer = NULL;
            model->text_buffer_size = 0;
            model->callback = NULL;
            model->callback_context = NULL;
        },
        true);
}

View* sim800_text_input_get_view(Sim800TextInput* text_input) {
    furi_assert(text_input);
    return text_input->view;
}

void sim800_text_input_set_result_callback(
    Sim800TextInput* text_input,
    Sim800TextInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text) {
    with_view_model(
        text_input->view,
        Sim800TextInputModel * model,
        {
            model->callback = callback;
            model->callback_context = callback_context;
            model->text_buffer = text_buffer;
            model->text_buffer_size = text_buffer_size;
            model->clear_default_text = clear_default_text;
            model->cursor_select = false;
            if(text_buffer && text_buffer[0] != '\0') {
                model->cursor_pos = strlen(text_buffer);
                // Start on Save, so a prefilled value is one press away
                model->selected_row = 2;
                model->selected_column = 9;
                model->selected_keyboard = 0;
            } else {
                model->cursor_pos = 0;
            }
        },
        true);
}

void sim800_text_input_set_minimum_length(Sim800TextInput* text_input, size_t minimum_length) {
    with_view_model(
        text_input->view,
        Sim800TextInputModel * model,
        { model->minimum_length = minimum_length; },
        true);
}

void sim800_text_input_set_header_text(Sim800TextInput* text_input, const char* text) {
    with_view_model(
        text_input->view, Sim800TextInputModel * model, { model->header = text; }, true);
}
