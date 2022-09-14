#include "byte_input.h"
#include <gui/elements.h>
#include <furi.h>

struct ByteInput {
    View* view;
};

typedef struct {
    const uint8_t value;
    const uint8_t x;
    const uint8_t y;
} ByteInputKey;

typedef struct {
    const char* header;
    uint8_t* bytes;
    uint8_t bytes_count;

    ByteInputCallback input_callback;
    ByteChangedCallback changed_callback;
    void* callback_context;

    bool selected_high_nibble;
    uint8_t selected_byte;
    int8_t selected_row; // row -1 - input, row 0 & 1 - keyboard
    uint8_t selected_column;
    uint8_t first_visible_byte;
} ByteInputModel;

static const uint8_t keyboard_origin_x = 7;
static const uint8_t keyboard_origin_y = 31;
static const uint8_t keyboard_row_count = 2;
static const uint8_t enter_symbol = '\r';
static const uint8_t backspace_symbol = '\b';
static const uint8_t max_drawable_bytes = 8;

static const ByteInputKey keyboard_keys_row_1[] = {
    {'0', 0, 12},
    {'1', 11, 12},
    {'2', 22, 12},
    {'3', 33, 12},
    {'4', 44, 12},
    {'5', 55, 12},
    {'6', 66, 12},
    {'7', 77, 12},
    {backspace_symbol, 103, 4},
};

static const ByteInputKey keyboard_keys_row_2[] = {
    {'8', 0, 26},
    {'9', 11, 26},
    {'A', 22, 26},
    {'B', 33, 26},
    {'C', 44, 26},
    {'D', 55, 26},
    {'E', 66, 26},
    {'F', 77, 26},
    {enter_symbol, 95, 17},
};

/**
 * @brief Get row size
 * 
 * @param row_index Index of row 
 * @return uint8_t Row size
 */
static uint8_t byte_input_get_row_size(uint8_t row_index) {
    uint8_t row_size = 0;

    switch(row_index + 1) {
    case 1:
        row_size = sizeof(keyboard_keys_row_1) / sizeof(ByteInputKey);
        break;
    case 2:
        row_size = sizeof(keyboard_keys_row_2) / sizeof(ByteInputKey);
        break;
    }

    return row_size;
}

/**
 * @brief Get row pointer
 * 
 * @param row_index Index of row 
 * @return const ByteInputKey* Row pointer
 */
static const ByteInputKey* byte_input_get_row(uint8_t row_index) {
    const ByteInputKey* row = NULL;

    switch(row_index + 1) {
    case 1:
        row = keyboard_keys_row_1;
        break;
    case 2:
        row = keyboard_keys_row_2;
        break;
    }

    return row;
}

/**
 * @brief Get text from nibble
 * 
 * @param byte byte value
 * @param high_nibble Get from high nibble, otherwise low nibble
 * @return char nibble text
 */
static char byte_input_get_nibble_text(uint8_t byte, bool high_nibble) {
    if(high_nibble) {
        byte = byte >> 4;
    }
    byte = byte & 0x0F;

    switch(byte & 0x0F) {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
    case 0x8:
    case 0x9:
        byte = byte + '0';
        break;
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
        byte = byte - 0xA + 'A';
        break;
    default:
        byte = '!';
        break;
    }

    return byte;
}

/**
 * @brief Draw input box (common view)
 * 
 * @param canvas 
 * @param model 
 */
static void byte_input_draw_input(Canvas* canvas, ByteInputModel* model) {
    const uint8_t text_x = 8;
    const uint8_t text_y = 25;

    elements_slightly_rounded_frame(canvas, 6, 14, 116, 15);

    canvas_draw_icon(canvas, 2, 19, &I_ButtonLeftSmall_3x5);
    canvas_draw_icon(canvas, 123, 19, &I_ButtonRightSmall_3x5);

    for(uint8_t i = model->first_visible_byte;
        i < model->first_visible_byte + MIN(model->bytes_count, max_drawable_bytes);
        i++) {
        uint8_t byte_position = i - model->first_visible_byte;

        if(i == model->selected_byte) {
            canvas_draw_frame(canvas, text_x + byte_position * 14, text_y - 9, 15, 11);

            if(model->selected_high_nibble) {
                canvas_draw_glyph(
                    canvas,
                    text_x + 8 + byte_position * 14,
                    text_y,
                    byte_input_get_nibble_text(model->bytes[i], false));
                canvas_draw_box(canvas, text_x + 1 + byte_position * 14, text_y - 8, 7, 9);
                canvas_invert_color(canvas);
                canvas_draw_line(
                    canvas,
                    text_x + 14 + byte_position * 14,
                    text_y - 6,
                    text_x + 14 + byte_position * 14,
                    text_y - 2);
                canvas_draw_glyph(
                    canvas,
                    text_x + 2 + byte_position * 14,
                    text_y,
                    byte_input_get_nibble_text(model->bytes[i], true));
                canvas_invert_color(canvas);
            } else {
                canvas_draw_box(canvas, text_x + 7 + byte_position * 14, text_y - 8, 7, 9);
                canvas_draw_glyph(
                    canvas,
                    text_x + 2 + byte_position * 14,
                    text_y,
                    byte_input_get_nibble_text(model->bytes[i], true));
                canvas_invert_color(canvas);
                canvas_draw_line(
                    canvas,
                    text_x + byte_position * 14,
                    text_y - 6,
                    text_x + byte_position * 14,
                    text_y - 2);
                canvas_draw_glyph(
                    canvas,
                    text_x + 8 + byte_position * 14,
                    text_y,
                    byte_input_get_nibble_text(model->bytes[i], false));
                canvas_invert_color(canvas);
            }
        } else {
            canvas_draw_glyph(
                canvas,
                text_x + 2 + byte_position * 14,
                text_y,
                byte_input_get_nibble_text(model->bytes[i], true));
            canvas_draw_glyph(
                canvas,
                text_x + 8 + byte_position * 14,
                text_y,
                byte_input_get_nibble_text(model->bytes[i], false));
        }
    }

    if(model->bytes_count - model->first_visible_byte > max_drawable_bytes) {
        canvas_draw_icon(canvas, 123, 21, &I_ButtonRightSmall_3x5);
    }

    if(model->first_visible_byte > 0) {
        canvas_draw_icon(canvas, 1, 21, &I_ButtonLeftSmall_3x5);
    }
}

/**
 * @brief Draw input box (selected view)
 * 
 * @param canvas 
 * @param model 
 */
static void byte_input_draw_input_selected(Canvas* canvas, ByteInputModel* model) {
    const uint8_t text_x = 7;
    const uint8_t text_y = 25;

    canvas_draw_box(canvas, 0, 12, 127, 19);
    canvas_invert_color(canvas);

    elements_slightly_rounded_frame(canvas, 6, 14, 115, 15);
    canvas_draw_icon(canvas, 2, 19, &I_ButtonLeftSmall_3x5);
    canvas_draw_icon(canvas, 122, 19, &I_ButtonRightSmall_3x5);

    for(uint8_t i = model->first_visible_byte;
        i < model->first_visible_byte + MIN(model->bytes_count, max_drawable_bytes);
        i++) {
        uint8_t byte_position = i - model->first_visible_byte;

        if(i == model->selected_byte) {
            canvas_draw_box(canvas, text_x + 1 + byte_position * 14, text_y - 9, 13, 11);
            canvas_invert_color(canvas);
            canvas_draw_glyph(
                canvas,
                text_x + 2 + byte_position * 14,
                text_y,
                byte_input_get_nibble_text(model->bytes[i], true));
            canvas_draw_glyph(
                canvas,
                text_x + 8 + byte_position * 14,
                text_y,
                byte_input_get_nibble_text(model->bytes[i], false));
            canvas_invert_color(canvas);
        } else {
            canvas_draw_glyph(
                canvas,
                text_x + 2 + byte_position * 14,
                text_y,
                byte_input_get_nibble_text(model->bytes[i], true));
            canvas_draw_glyph(
                canvas,
                text_x + 8 + byte_position * 14,
                text_y,
                byte_input_get_nibble_text(model->bytes[i], false));
        }
    }

    if(model->bytes_count - model->first_visible_byte > max_drawable_bytes) {
        canvas_draw_icon(canvas, 123, 21, &I_ButtonRightSmall_3x5);
    }

    if(model->first_visible_byte > 0) {
        canvas_draw_icon(canvas, 1, 21, &I_ButtonLeftSmall_3x5);
    }

    canvas_invert_color(canvas);
}

/**
 * @brief Set nibble at position
 * 
 * @param data where to set nibble
 * @param position byte position
 * @param value char value
 * @param high_nibble set high nibble
 */
static void byte_input_set_nibble(uint8_t* data, uint8_t position, char value, bool high_nibble) {
    switch(value) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        value = value - '0';
        break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
        value = value - 'A' + 10;
        break;
    default:
        value = 0;
        break;
    }

    if(high_nibble) {
        data[position] &= 0x0F;
        data[position] |= value << 4;
    } else {
        data[position] &= 0xF0;
        data[position] |= value;
    }
}

/**
 * @brief What currently selected
 * 
 * @return true - keyboard selected, false - input selected
 */
static bool byte_input_keyboard_selected(ByteInputModel* model) {
    return model->selected_row >= 0;
}

/**
 * @brief Do transition from keyboard
 * 
 * @param model 
 */
static void byte_input_transition_from_keyboard(ByteInputModel* model) {
    model->selected_row += 1;
    model->selected_high_nibble = true;
}

/**
 * @brief Increase selected byte position
 * 
 * @param model 
 */
static void byte_input_inc_selected_byte(ByteInputModel* model) {
    if(model->selected_byte < model->bytes_count - 1) {
        model->selected_byte += 1;

        if(model->bytes_count > max_drawable_bytes) {
            if(model->selected_byte - model->first_visible_byte > (max_drawable_bytes - 2)) {
                if(model->first_visible_byte < model->bytes_count - max_drawable_bytes) {
                    model->first_visible_byte++;
                }
            }
        }
    }
}

/**
 * @brief Decrease selected byte position
 * 
 * @param model 
 */
static void byte_input_dec_selected_byte(ByteInputModel* model) {
    if(model->selected_byte > 0) {
        model->selected_byte -= 1;

        if(model->selected_byte - model->first_visible_byte < 1) {
            if(model->first_visible_byte > 0) {
                model->first_visible_byte--;
            }
        }
    }
}

/**
 * @brief Call input callback
 * 
 * @param model 
 */
static void byte_input_call_input_callback(ByteInputModel* model) {
    if(model->input_callback != NULL) {
        model->input_callback(model->callback_context);
    }
}

/**
 * @brief Call changed callback
 * 
 * @param model 
 */
static void byte_input_call_changed_callback(ByteInputModel* model) {
    if(model->changed_callback != NULL) {
        model->changed_callback(model->callback_context);
    }
}

/**
 * @brief Clear selected byte 
 */

static void byte_input_clear_selected_byte(ByteInputModel* model) {
    model->bytes[model->selected_byte] = 0;
    model->selected_high_nibble = true;
    byte_input_dec_selected_byte(model);
    byte_input_call_changed_callback(model);
}

/**
 * @brief Handle up button
 * 
 * @param model 
 */
static void byte_input_handle_up(ByteInputModel* model) {
    if(model->selected_row > -1) {
        model->selected_row -= 1;
    }
}

/**
 * @brief Handle down button
 * 
 * @param model 
 */
static void byte_input_handle_down(ByteInputModel* model) {
    if(byte_input_keyboard_selected(model)) {
        if(model->selected_row < keyboard_row_count - 1) {
            model->selected_row += 1;
        }
    } else {
        byte_input_transition_from_keyboard(model);
    }
}

/**
 * @brief Handle left button
 * 
 * @param model 
 */
static void byte_input_handle_left(ByteInputModel* model) {
    if(byte_input_keyboard_selected(model)) {
        if(model->selected_column > 0) {
            model->selected_column -= 1;
        } else {
            model->selected_column = byte_input_get_row_size(model->selected_row) - 1;
        }
    } else {
        byte_input_dec_selected_byte(model);
    }
}

/**
 * @brief Handle right button
 * 
 * @param model 
 */
static void byte_input_handle_right(ByteInputModel* model) {
    if(byte_input_keyboard_selected(model)) {
        if(model->selected_column < byte_input_get_row_size(model->selected_row) - 1) {
            model->selected_column += 1;
        } else {
            model->selected_column = 0;
        }
    } else {
        byte_input_inc_selected_byte(model);
    }
}

/**
 * @brief Handle OK button
 * 
 * @param model 
 */
static void byte_input_handle_ok(ByteInputModel* model) {
    if(byte_input_keyboard_selected(model)) {
        uint8_t value = byte_input_get_row(model->selected_row)[model->selected_column].value;

        if(value == enter_symbol) {
            byte_input_call_input_callback(model);
        } else if(value == backspace_symbol) {
            byte_input_clear_selected_byte(model);
        } else {
            byte_input_set_nibble(
                model->bytes, model->selected_byte, value, model->selected_high_nibble);
            if(model->selected_high_nibble == true) {
                model->selected_high_nibble = false;
            } else {
                byte_input_inc_selected_byte(model);
                model->selected_high_nibble = true;
            }
            byte_input_call_changed_callback(model);
        }
    } else {
        byte_input_transition_from_keyboard(model);
    }
}

/**
 * @brief Draw callback
 * 
 * @param canvas 
 * @param _model 
 */
static void byte_input_view_draw_callback(Canvas* canvas, void* _model) {
    ByteInputModel* model = _model;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_str(canvas, 2, 9, model->header);

    canvas_set_font(canvas, FontKeyboard);

    if(model->selected_row == -1) {
        byte_input_draw_input_selected(canvas, model);
    } else {
        byte_input_draw_input(canvas, model);
    }

    for(uint8_t row = 0; row < keyboard_row_count; row++) {
        const uint8_t column_count = byte_input_get_row_size(row);
        const ByteInputKey* keys = byte_input_get_row(row);

        for(size_t column = 0; column < column_count; column++) {
            if(keys[column].value == enter_symbol) {
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
            } else if(keys[column].value == backspace_symbol) {
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
                        keyboard_origin_x + keys[column].x - 3,
                        keyboard_origin_y + keys[column].y - 10,
                        11,
                        13);
                    canvas_set_color(canvas, ColorWhite);
                } else if(model->selected_row == -1 && row == 0 && model->selected_column == column) {
                    canvas_set_color(canvas, ColorBlack);
                    canvas_draw_frame(
                        canvas,
                        keyboard_origin_x + keys[column].x - 3,
                        keyboard_origin_y + keys[column].y - 10,
                        11,
                        13);
                } else {
                    canvas_set_color(canvas, ColorBlack);
                }

                canvas_draw_glyph(
                    canvas,
                    keyboard_origin_x + keys[column].x,
                    keyboard_origin_y + keys[column].y,
                    keys[column].value);
            }
        }
    }
}

/**
 * @brief Input callback
 * 
 * @param event 
 * @param context 
 * @return true 
 * @return false 
 */
static bool byte_input_view_input_callback(InputEvent* event, void* context) {
    ByteInput* byte_input = context;
    furi_assert(byte_input);
    bool consumed = false;

    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
        case InputKeyLeft:
            with_view_model(
                byte_input->view, (ByteInputModel * model) {
                    byte_input_handle_left(model);
                    return true;
                });
            consumed = true;
            break;
        case InputKeyRight:
            with_view_model(
                byte_input->view, (ByteInputModel * model) {
                    byte_input_handle_right(model);
                    return true;
                });
            consumed = true;
            break;
        case InputKeyUp:
            with_view_model(
                byte_input->view, (ByteInputModel * model) {
                    byte_input_handle_up(model);
                    return true;
                });
            consumed = true;
            break;
        case InputKeyDown:
            with_view_model(
                byte_input->view, (ByteInputModel * model) {
                    byte_input_handle_down(model);
                    return true;
                });
            consumed = true;
            break;
        case InputKeyOk:
            with_view_model(
                byte_input->view, (ByteInputModel * model) {
                    byte_input_handle_ok(model);
                    return true;
                });
            consumed = true;
            break;
        default:
            break;
        }
    }

    if((event->type == InputTypeLong || event->type == InputTypeRepeat) &&
       event->key == InputKeyBack) {
        with_view_model(
            byte_input->view, (ByteInputModel * model) {
                byte_input_clear_selected_byte(model);
                return true;
            });
        consumed = true;
    }

    return consumed;
}

/**
 * @brief Reset all input-related data in model
 * 
 * @param model ByteInputModel
 */
static void byte_input_reset_model_input_data(ByteInputModel* model) {
    model->bytes = NULL;
    model->bytes_count = 0;
    model->selected_high_nibble = true;
    model->selected_byte = 0;
    model->selected_row = 0;
    model->selected_column = 0;
    model->first_visible_byte = 0;
}

/** 
 * @brief Allocate and initialize byte input. This byte input is used to enter bytes.
 * 
 * @return ByteInput instance pointer
 */
ByteInput* byte_input_alloc() {
    ByteInput* byte_input = malloc(sizeof(ByteInput));
    byte_input->view = view_alloc();
    view_set_context(byte_input->view, byte_input);
    view_allocate_model(byte_input->view, ViewModelTypeLocking, sizeof(ByteInputModel));
    view_set_draw_callback(byte_input->view, byte_input_view_draw_callback);
    view_set_input_callback(byte_input->view, byte_input_view_input_callback);

    with_view_model(
        byte_input->view, (ByteInputModel * model) {
            model->header = "";
            model->input_callback = NULL;
            model->changed_callback = NULL;
            model->callback_context = NULL;
            byte_input_reset_model_input_data(model);
            return true;
        });

    return byte_input;
}

/** 
 * @brief Deinitialize and free byte input
 * 
 * @param byte_input Byte input instance
 */
void byte_input_free(ByteInput* byte_input) {
    furi_assert(byte_input);
    view_free(byte_input->view);
    free(byte_input);
}

/** 
 * @brief Get byte input view
 * 
 * @param byte_input byte input instance
 * @return View instance that can be used for embedding
 */
View* byte_input_get_view(ByteInput* byte_input) {
    furi_assert(byte_input);
    return byte_input->view;
}

/** 
 * @brief Deinitialize and free byte input
 * 
 * @param byte_input byte input instance
 * @param input_callback input callback fn
 * @param changed_callback changed callback fn
 * @param callback_context callback context
 * @param bytes buffer to use
 * @param bytes_count buffer length
 */
void byte_input_set_result_callback(
    ByteInput* byte_input,
    ByteInputCallback input_callback,
    ByteChangedCallback changed_callback,
    void* callback_context,
    uint8_t* bytes,
    uint8_t bytes_count) {
    with_view_model(
        byte_input->view, (ByteInputModel * model) {
            byte_input_reset_model_input_data(model);
            model->input_callback = input_callback;
            model->changed_callback = changed_callback;
            model->callback_context = callback_context;
            model->bytes = bytes;
            model->bytes_count = bytes_count;
            return true;
        });
}

/**
 * @brief Set byte input header text
 * 
 * @param byte_input byte input instance
 * @param text text to be shown
 */
void byte_input_set_header_text(ByteInput* byte_input, const char* text) {
    with_view_model(
        byte_input->view, (ByteInputModel * model) {
            model->header = text;
            return true;
        });
}
