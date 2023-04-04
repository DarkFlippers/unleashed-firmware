#include "uart_text_input.h"
#include <gui/elements.h>
#include "uart_terminal_icons.h"
#include "uart_terminal_app_i.h"
#include <furi.h>

struct UART_TextInput {
    View* view;
    FuriTimer* timer;
};

typedef struct {
    const char text;
    const uint8_t x;
    const uint8_t y;
} UART_TextInputKey;

typedef struct {
    const char* header;
    char* text_buffer;
    size_t text_buffer_size;
    bool clear_default_text;

    UART_TextInputCallback callback;
    void* callback_context;

    uint8_t selected_row;
    uint8_t selected_column;

    UART_TextInputValidatorCallback validator_callback;
    void* validator_callback_context;
    FuriString* validator_text;
    bool valadator_message_visible;
} UART_TextInputModel;

static const uint8_t keyboard_origin_x = 1;
static const uint8_t keyboard_origin_y = 29;
static const uint8_t keyboard_row_count = 4;

#define mode_AT "Send AT command to UART"

#define ENTER_KEY '\r'
#define BACKSPACE_KEY '\b'

static const UART_TextInputKey keyboard_keys_row_1[] = {
    {'{', 1, 0},
    {'(', 9, 0},
    {'[', 17, 0},
    {'|', 25, 0},
    {'@', 33, 0},
    {'&', 41, 0},
    {'#', 49, 0},
    {';', 57, 0},
    {'^', 65, 0},
    {'*', 73, 0},
    {'`', 81, 0},
    {'"', 89, 0},
    {'~', 97, 0},
    {'\'', 105, 0},
    {'.', 113, 0},
    {'/', 120, 0},
};

static const UART_TextInputKey keyboard_keys_row_2[] = {
    {'q', 1, 10},
    {'w', 9, 10},
    {'e', 17, 10},
    {'r', 25, 10},
    {'t', 33, 10},
    {'y', 41, 10},
    {'u', 49, 10},
    {'i', 57, 10},
    {'o', 65, 10},
    {'p', 73, 10},
    {'0', 81, 10},
    {'1', 89, 10},
    {'2', 97, 10},
    {'3', 105, 10},
    {'=', 113, 10},
    {'-', 120, 10},
};

static const UART_TextInputKey keyboard_keys_row_3[] = {
    {'a', 1, 21},
    {'s', 9, 21},
    {'d', 18, 21},
    {'f', 25, 21},
    {'g', 33, 21},
    {'h', 41, 21},
    {'j', 49, 21},
    {'k', 57, 21},
    {'l', 65, 21},
    {BACKSPACE_KEY, 72, 13},
    {'4', 89, 21},
    {'5', 97, 21},
    {'6', 105, 21},
    {'$', 113, 21},
    {'%', 120, 21},

};

static const UART_TextInputKey keyboard_keys_row_4[] = {
    {'z', 1, 33},
    {'x', 9, 33},
    {'c', 18, 33},
    {'v', 25, 33},
    {'b', 33, 33},
    {'n', 41, 33},
    {'m', 49, 33},
    {'_', 57, 33},
    {ENTER_KEY, 64, 24},
    {'7', 89, 33},
    {'8', 97, 33},
    {'9', 105, 33},
    {'!', 113, 33},
    {'+', 120, 33},
};

static uint8_t get_row_size(uint8_t row_index) {
    uint8_t row_size = 0;

    switch(row_index + 1) {
    case 1:
        row_size = sizeof(keyboard_keys_row_1) / sizeof(UART_TextInputKey);
        break;
    case 2:
        row_size = sizeof(keyboard_keys_row_2) / sizeof(UART_TextInputKey);
        break;
    case 3:
        row_size = sizeof(keyboard_keys_row_3) / sizeof(UART_TextInputKey);
        break;
    case 4:
        row_size = sizeof(keyboard_keys_row_4) / sizeof(UART_TextInputKey);
        break;
    }

    return row_size;
}

static const UART_TextInputKey* get_row(uint8_t row_index) {
    const UART_TextInputKey* row = NULL;

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
    case 4:
        row = keyboard_keys_row_4;
        break;
    }

    return row;
}

static char get_selected_char(UART_TextInputModel* model) {
    return get_row(model->selected_row)[model->selected_column].text;
}

static bool char_is_lowercase(char letter) {
    return (letter >= 0x61 && letter <= 0x7A);
}

static bool char_is_uppercase(char letter) {
    return (letter >= 0x41 && letter <= 0x5A);
}

static char char_to_lowercase(const char letter) {
    switch(letter) {
    case ' ':
        return 0x5f;
        break;
    case ')':
        return 0x28;
        break;
    case '}':
        return 0x7b;
        break;
    case ']':
        return 0x5b;
        break;
    case '\\':
        return 0x2f;
        break;
    case ':':
        return 0x3b;
        break;
    case ',':
        return 0x2e;
        break;
    case '?':
        return 0x21;
        break;
    case '>':
        return 0x3c;
        break;
    }
    if(char_is_uppercase(letter)) {
        return (letter + 0x20);
    } else {
        return letter;
    }
}

static char char_to_uppercase(const char letter) {
    switch(letter) {
    case '_':
        return 0x20;
        break;
    case '(':
        return 0x29;
        break;
    case '{':
        return 0x7d;
        break;
    case '[':
        return 0x5d;
        break;
    case '/':
        return 0x5c;
        break;
    case ';':
        return 0x3a;
        break;
    case '.':
        return 0x2c;
        break;
    case '!':
        return 0x3f;
        break;
    case '<':
        return 0x3e;
        break;
    }
    if(char_is_lowercase(letter)) {
        return (letter - 0x20);
    } else {
        return letter;
    }
}

static void uart_text_input_backspace_cb(UART_TextInputModel* model) {
    uint8_t text_length = model->clear_default_text ? 1 : strlen(model->text_buffer);
    if(text_length > 0) {
        model->text_buffer[text_length - 1] = 0;
    }
}

static void uart_text_input_view_draw_callback(Canvas* canvas, void* _model) {
    UART_TextInputModel* model = _model;
    //uint8_t text_length = model->text_buffer ? strlen(model->text_buffer) : 0;
    uint8_t needed_string_width = canvas_width(canvas) - 8;
    uint8_t start_pos = 4;

    const char* text = model->text_buffer;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_str(canvas, 2, 7, model->header);
    elements_slightly_rounded_frame(canvas, 1, 8, 126, 12);

    if(canvas_string_width(canvas, text) > needed_string_width) {
        canvas_draw_str(canvas, start_pos, 17, "...");
        start_pos += 6;
        needed_string_width -= 8;
    }

    while(text != 0 && canvas_string_width(canvas, text) > needed_string_width) {
        text++;
    }

    if(model->clear_default_text) {
        elements_slightly_rounded_box(
            canvas, start_pos - 1, 14, canvas_string_width(canvas, text) + 2, 10);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_draw_str(canvas, start_pos + canvas_string_width(canvas, text) + 1, 18, "|");
        canvas_draw_str(canvas, start_pos + canvas_string_width(canvas, text) + 2, 18, "|");
    }
    canvas_draw_str(canvas, start_pos, 17, text);

    canvas_set_font(canvas, FontKeyboard);

    for(uint8_t row = 0; row <= keyboard_row_count; row++) {
        const uint8_t column_count = get_row_size(row);
        const UART_TextInputKey* keys = get_row(row);

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
                if(0 == strcmp(model->header, mode_AT)) {
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
    if(model->valadator_message_visible) {
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

static void
    uart_text_input_handle_up(UART_TextInput* uart_text_input, UART_TextInputModel* model) {
    UNUSED(uart_text_input);
    if(model->selected_row > 0) {
        model->selected_row--;
        if(model->selected_column > get_row_size(model->selected_row) - 6) {
            model->selected_column = model->selected_column + 1;
        }
    }
}

static void
    uart_text_input_handle_down(UART_TextInput* uart_text_input, UART_TextInputModel* model) {
    UNUSED(uart_text_input);
    if(model->selected_row < keyboard_row_count - 1) {
        model->selected_row++;
        if(model->selected_column > get_row_size(model->selected_row) - 4) {
            model->selected_column = model->selected_column - 1;
        }
    }
}

static void
    uart_text_input_handle_left(UART_TextInput* uart_text_input, UART_TextInputModel* model) {
    UNUSED(uart_text_input);
    if(model->selected_column > 0) {
        model->selected_column--;
    } else {
        model->selected_column = get_row_size(model->selected_row) - 1;
    }
}

static void
    uart_text_input_handle_right(UART_TextInput* uart_text_input, UART_TextInputModel* model) {
    UNUSED(uart_text_input);
    if(model->selected_column < get_row_size(model->selected_row) - 1) {
        model->selected_column++;
    } else {
        model->selected_column = 0;
    }
}

static void uart_text_input_handle_ok(
    UART_TextInput* uart_text_input,
    UART_TextInputModel* model,
    bool shift) {
    char selected = get_selected_char(model);
    uint8_t text_length = strlen(model->text_buffer);

    if(0 == strcmp(model->header, mode_AT)) {
        selected = char_to_uppercase(selected);
    }

    if(shift) {
        if(0 == strcmp(model->header, mode_AT)) {
            selected = char_to_lowercase(selected);
        } else {
            selected = char_to_uppercase(selected);
        }
    }

    if(selected == ENTER_KEY) {
        if(model->validator_callback &&
           (!model->validator_callback(
               model->text_buffer, model->validator_text, model->validator_callback_context))) {
            model->valadator_message_visible = true;
            furi_timer_start(uart_text_input->timer, furi_kernel_get_tick_frequency() * 4);
        } else if(model->callback != 0 && text_length > 0) {
            model->callback(model->callback_context);
        }
    } else if(selected == BACKSPACE_KEY) {
        uart_text_input_backspace_cb(model);
    } else {
        if(model->clear_default_text) {
            text_length = 0;
        }
        if(text_length < (model->text_buffer_size - 1)) {
            model->text_buffer[text_length] = selected;
            model->text_buffer[text_length + 1] = 0;
        }
    }
    model->clear_default_text = false;
}

static bool uart_text_input_view_input_callback(InputEvent* event, void* context) {
    UART_TextInput* uart_text_input = context;
    furi_assert(uart_text_input);

    bool consumed = false;

    // Acquire model
    UART_TextInputModel* model = view_get_model(uart_text_input->view);

    if((!(event->type == InputTypePress) && !(event->type == InputTypeRelease)) &&
       model->valadator_message_visible) {
        model->valadator_message_visible = false;
        consumed = true;
    } else if(event->type == InputTypeShort) {
        consumed = true;
        switch(event->key) {
        case InputKeyUp:
            uart_text_input_handle_up(uart_text_input, model);
            break;
        case InputKeyDown:
            uart_text_input_handle_down(uart_text_input, model);
            break;
        case InputKeyLeft:
            uart_text_input_handle_left(uart_text_input, model);
            break;
        case InputKeyRight:
            uart_text_input_handle_right(uart_text_input, model);
            break;
        case InputKeyOk:
            uart_text_input_handle_ok(uart_text_input, model, false);
            break;
        default:
            consumed = false;
            break;
        }
    } else if(event->type == InputTypeLong) {
        consumed = true;
        switch(event->key) {
        case InputKeyUp:
            uart_text_input_handle_up(uart_text_input, model);
            break;
        case InputKeyDown:
            uart_text_input_handle_down(uart_text_input, model);
            break;
        case InputKeyLeft:
            uart_text_input_handle_left(uart_text_input, model);
            break;
        case InputKeyRight:
            uart_text_input_handle_right(uart_text_input, model);
            break;
        case InputKeyOk:
            uart_text_input_handle_ok(uart_text_input, model, true);
            break;
        case InputKeyBack:
            uart_text_input_backspace_cb(model);
            break;
        default:
            consumed = false;
            break;
        }
    } else if(event->type == InputTypeRepeat) {
        consumed = true;
        switch(event->key) {
        case InputKeyUp:
            uart_text_input_handle_up(uart_text_input, model);
            break;
        case InputKeyDown:
            uart_text_input_handle_down(uart_text_input, model);
            break;
        case InputKeyLeft:
            uart_text_input_handle_left(uart_text_input, model);
            break;
        case InputKeyRight:
            uart_text_input_handle_right(uart_text_input, model);
            break;
        case InputKeyBack:
            uart_text_input_backspace_cb(model);
            break;
        default:
            consumed = false;
            break;
        }
    }

    // Commit model
    view_commit_model(uart_text_input->view, consumed);

    return consumed;
}

void uart_text_input_timer_callback(void* context) {
    furi_assert(context);
    UART_TextInput* uart_text_input = context;

    with_view_model(
        uart_text_input->view,
        UART_TextInputModel * model,
        { model->valadator_message_visible = false; },
        true);
}

UART_TextInput* uart_text_input_alloc() {
    UART_TextInput* uart_text_input = malloc(sizeof(UART_TextInput));
    uart_text_input->view = view_alloc();
    view_set_context(uart_text_input->view, uart_text_input);
    view_allocate_model(uart_text_input->view, ViewModelTypeLocking, sizeof(UART_TextInputModel));
    view_set_draw_callback(uart_text_input->view, uart_text_input_view_draw_callback);
    view_set_input_callback(uart_text_input->view, uart_text_input_view_input_callback);

    uart_text_input->timer =
        furi_timer_alloc(uart_text_input_timer_callback, FuriTimerTypeOnce, uart_text_input);

    with_view_model(
        uart_text_input->view,
        UART_TextInputModel * model,
        { model->validator_text = furi_string_alloc(); },
        false);

    uart_text_input_reset(uart_text_input);

    return uart_text_input;
}

void uart_text_input_free(UART_TextInput* uart_text_input) {
    furi_assert(uart_text_input);
    with_view_model(
        uart_text_input->view,
        UART_TextInputModel * model,
        { furi_string_free(model->validator_text); },
        false);

    // Send stop command
    furi_timer_stop(uart_text_input->timer);
    // Release allocated memory
    furi_timer_free(uart_text_input->timer);

    view_free(uart_text_input->view);

    free(uart_text_input);
}

void uart_text_input_reset(UART_TextInput* uart_text_input) {
    furi_assert(uart_text_input);
    with_view_model(
        uart_text_input->view,
        UART_TextInputModel * model,
        {
            model->text_buffer_size = 0;
            model->header = "";
            model->selected_row = 0;
            model->selected_column = 0;
            model->clear_default_text = false;
            model->text_buffer = NULL;
            model->text_buffer_size = 0;
            model->callback = NULL;
            model->callback_context = NULL;
            model->validator_callback = NULL;
            model->validator_callback_context = NULL;
            furi_string_reset(model->validator_text);
            model->valadator_message_visible = false;
        },
        true);
}

View* uart_text_input_get_view(UART_TextInput* uart_text_input) {
    furi_assert(uart_text_input);
    return uart_text_input->view;
}

void uart_text_input_set_result_callback(
    UART_TextInput* uart_text_input,
    UART_TextInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text) {
    with_view_model(
        uart_text_input->view,
        UART_TextInputModel * model,
        {
            model->callback = callback;
            model->callback_context = callback_context;
            model->text_buffer = text_buffer;
            model->text_buffer_size = text_buffer_size;
            model->clear_default_text = clear_default_text;
            if(text_buffer && text_buffer[0] != '\0') {
                // Set focus on Save
                model->selected_row = 2;
                model->selected_column = 8;
            }
        },
        true);
}

void uart_text_input_set_validator(
    UART_TextInput* uart_text_input,
    UART_TextInputValidatorCallback callback,
    void* callback_context) {
    with_view_model(
        uart_text_input->view,
        UART_TextInputModel * model,
        {
            model->validator_callback = callback;
            model->validator_callback_context = callback_context;
        },
        true);
}

UART_TextInputValidatorCallback
    uart_text_input_get_validator_callback(UART_TextInput* uart_text_input) {
    UART_TextInputValidatorCallback validator_callback = NULL;
    with_view_model(
        uart_text_input->view,
        UART_TextInputModel * model,
        { validator_callback = model->validator_callback; },
        false);
    return validator_callback;
}

void* uart_text_input_get_validator_callback_context(UART_TextInput* uart_text_input) {
    void* validator_callback_context = NULL;
    with_view_model(
        uart_text_input->view,
        UART_TextInputModel * model,
        { validator_callback_context = model->validator_callback_context; },
        false);
    return validator_callback_context;
}

void uart_text_input_set_header_text(UART_TextInput* uart_text_input, const char* text) {
    with_view_model(
        uart_text_input->view, UART_TextInputModel * model, { model->header = text; }, true);
}
