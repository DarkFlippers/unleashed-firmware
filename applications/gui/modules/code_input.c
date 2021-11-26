#include "code_input.h"
#include <gui/elements.h>
#include <furi.h>

#define MAX_CODE_LEN 10

struct CodeInput {
    View* view;
};

typedef enum {
    CodeInputStateVerify,
    CodeInputStateUpdate,
    CodeInputStateTotal,
} CodeInputStateEnum;

typedef enum {
    CodeInputFirst,
    CodeInputSecond,
    CodeInputTotal,
} CodeInputsEnum;

typedef struct {
    uint8_t state;
    uint8_t current;
    bool ext_update;

    uint8_t input_length[CodeInputTotal];
    uint8_t local_buffer[CodeInputTotal][MAX_CODE_LEN];

    CodeInputOkCallback ok_callback;
    CodeInputFailCallback fail_callback;
    void* callback_context;

    const char* header;

    uint8_t* ext_buffer;
    uint8_t* ext_buffer_length;
} CodeInputModel;

static const Icon* keys_assets[] = {
    [InputKeyUp] = &I_ButtonUp_7x4,
    [InputKeyDown] = &I_ButtonDown_7x4,
    [InputKeyRight] = &I_ButtonRight_4x7,
    [InputKeyLeft] = &I_ButtonLeft_4x7,
};

/**
 * @brief Compare buffers
 * 
 * @param in Input buffer pointer
 * @param len_in Input array length
 * @param src Source buffer pointer
 * @param len_src Source array length
 */

bool code_input_compare(uint8_t* in, size_t len_in, uint8_t* src, size_t len_src) {
    bool result = false;
    do {
        result = (len_in && len_src);
        if(!result) {
            break;
        }
        result = (len_in == len_src);
        if(!result) {
            break;
        }
        for(size_t i = 0; i < len_in; i++) {
            result = (in[i] == src[i]);
            if(!result) {
                break;
            }
        }
    } while(false);

    return result;
}

/**
 * @brief Compare local buffers
 * 
 * @param model 
 */
static bool code_input_compare_local(CodeInputModel* model) {
    uint8_t* source = model->local_buffer[CodeInputFirst];
    size_t source_length = model->input_length[CodeInputFirst];

    uint8_t* input = model->local_buffer[CodeInputSecond];
    size_t input_length = model->input_length[CodeInputSecond];

    return code_input_compare(input, input_length, source, source_length);
}

/**
 * @brief Compare ext with local
 * 
 * @param model 
 */
static bool code_input_compare_ext(CodeInputModel* model) {
    uint8_t* input = model->local_buffer[CodeInputFirst];
    size_t input_length = model->input_length[CodeInputFirst];

    uint8_t* source = model->ext_buffer;
    size_t source_length = *model->ext_buffer_length;

    return code_input_compare(input, input_length, source, source_length);
}

/**
 * @brief Set ext buffer
 * 
 * @param model 
 */
static void code_input_set_ext(CodeInputModel* model) {
    *model->ext_buffer_length = model->input_length[CodeInputFirst];
    for(size_t i = 0; i <= model->input_length[CodeInputFirst]; i++) {
        model->ext_buffer[i] = model->local_buffer[CodeInputFirst][i];
    }
}

/**
 * @brief Draw input sequence
 * 
 * @param canvas 
 * @param buffer 
 * @param length 
 * @param x 
 * @param y 
 * @param active
 */
static void code_input_draw_sequence(
    Canvas* canvas,
    uint8_t* buffer,
    uint8_t length,
    uint8_t x,
    uint8_t y,
    bool active) {
    uint8_t pos_x = x + 6;
    uint8_t pos_y = y + 3;

    if(active) canvas_draw_icon(canvas, x - 4, y + 5, &I_ButtonRightSmall_3x5);

    elements_slightly_rounded_frame(canvas, x, y, 116, 15);

    for(size_t i = 0; i < length; i++) {
        // maybe symmetrical assets? :-/
        uint8_t offset_y = buffer[i] < 2 ? 2 + (buffer[i] * 2) : 1;
        canvas_draw_icon(canvas, pos_x, pos_y + offset_y, keys_assets[buffer[i]]);
        pos_x += buffer[i] > 1 ? 9 : 11;
    }
}

/**
 * @brief Reset input count
 * 
 * @param model 
 */
static void code_input_reset_count(CodeInputModel* model) {
    model->input_length[model->current] = 0;
}

/**
 * @brief Call input callback
 * 
 * @param model 
 */
static void code_input_call_ok_callback(CodeInputModel* model) {
    if(model->ok_callback != NULL) {
        model->ok_callback(model->callback_context);
    }
}

/**
 * @brief Call changed callback
 * 
 * @param model 
 */
static void code_input_call_fail_callback(CodeInputModel* model) {
    if(model->fail_callback != NULL) {
        model->fail_callback(model->callback_context);
    }
}

/**
 * @brief Handle Back button
 * 
 * @param model 
 */
static bool code_input_handle_back(CodeInputModel* model) {
    if(model->current && !model->input_length[model->current]) {
        --model->current;
        return true;
    }

    if(model->input_length[model->current]) {
        code_input_reset_count(model);
        return true;
    }

    code_input_call_fail_callback(model);
    return false;
}

/**
 * @brief Handle OK button
 * 
 * @param model 
 */
static void code_input_handle_ok(CodeInputModel* model) {
    switch(model->state) {
    case CodeInputStateVerify:

        if(code_input_compare_ext(model)) {
            if(model->ext_update) {
                model->state = CodeInputStateUpdate;
            } else {
                code_input_call_ok_callback(model);
            }
        }
        code_input_reset_count(model);
        break;

    case CodeInputStateUpdate:

        if(!model->current && model->input_length[model->current]) {
            model->current++;
        } else {
            if(code_input_compare_local(model)) {
                if(model->ext_update) {
                    code_input_set_ext(model);
                }
                code_input_call_ok_callback(model);
            } else {
                code_input_reset_count(model);
            }
        }

        break;
    default:
        break;
    }
}

/**
 * @brief Handle input
 * 
 * @param model 
 * @param key 
 */

size_t code_input_push(uint8_t* buffer, size_t length, InputKey key) {
    buffer[length] = key;
    length = CLAMP(length + 1, MAX_CODE_LEN, 0);
    return length;
}

/**
 * @brief Handle D-pad keys
 * 
 * @param model 
 * @param key 
 */
static void code_input_handle_dpad(CodeInputModel* model, InputKey key) {
    uint8_t at = model->current;
    size_t new_length = code_input_push(model->local_buffer[at], model->input_length[at], key);
    model->input_length[at] = new_length;
}

/**
 * @brief Draw callback
 * 
 * @param canvas 
 * @param _model 
 */
static void code_input_view_draw_callback(Canvas* canvas, void* _model) {
    CodeInputModel* model = _model;
    uint8_t y_offset = 0;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    if(model->header && strlen(model->header)) {
        canvas_draw_str(canvas, 2, 9, model->header);
    } else {
        y_offset = 4;
    }

    canvas_set_font(canvas, FontSecondary);

    switch(model->state) {
    case CodeInputStateVerify:
        code_input_draw_sequence(
            canvas,
            model->local_buffer[CodeInputFirst],
            model->input_length[CodeInputFirst],
            6,
            30 + y_offset,
            true);
        break;
    case CodeInputStateUpdate:
        code_input_draw_sequence(
            canvas,
            model->local_buffer[CodeInputFirst],
            model->input_length[CodeInputFirst],
            6,
            14 + y_offset,
            !model->current);
        code_input_draw_sequence(
            canvas,
            model->local_buffer[CodeInputSecond],
            model->input_length[CodeInputSecond],
            6,
            44 + y_offset,
            model->current);

        if(model->current) canvas_draw_str(canvas, 2, 39 - y_offset, "Repeat code");

        break;
    default:
        break;
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
static bool code_input_view_input_callback(InputEvent* event, void* context) {
    CodeInput* code_input = context;
    furi_assert(code_input);
    bool consumed = false;

    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
        case InputKeyBack:
            with_view_model(
                code_input->view, (CodeInputModel * model) {
                    consumed = code_input_handle_back(model);
                    return true;
                });
            break;

        case InputKeyOk:
            with_view_model(
                code_input->view, (CodeInputModel * model) {
                    code_input_handle_ok(model);
                    return true;
                });
            consumed = true;
            break;
        default:

            with_view_model(
                code_input->view, (CodeInputModel * model) {
                    code_input_handle_dpad(model, event->key);
                    return true;
                });
            consumed = true;
            break;
        }
    }

    return consumed;
}

/**
 * @brief Reset all input-related data in model
 * 
 * @param model CodeInputModel
 */
static void code_input_reset_model_input_data(CodeInputModel* model) {
    model->current = 0;
    model->input_length[CodeInputFirst] = 0;
    model->input_length[CodeInputSecond] = 0;
    model->ext_buffer = NULL;
    model->ext_update = false;
    model->state = 0;
}

/** 
 * @brief Allocate and initialize code input. This code input is used to enter codes.
 * 
 * @return CodeInput instance pointer
 */
CodeInput* code_input_alloc() {
    CodeInput* code_input = furi_alloc(sizeof(CodeInput));
    code_input->view = view_alloc();
    view_set_context(code_input->view, code_input);
    view_allocate_model(code_input->view, ViewModelTypeLocking, sizeof(CodeInputModel));
    view_set_draw_callback(code_input->view, code_input_view_draw_callback);
    view_set_input_callback(code_input->view, code_input_view_input_callback);

    with_view_model(
        code_input->view, (CodeInputModel * model) {
            model->header = "";
            model->ok_callback = NULL;
            model->fail_callback = NULL;
            model->callback_context = NULL;
            code_input_reset_model_input_data(model);
            return true;
        });

    return code_input;
}

/** 
 * @brief Deinitialize and free code input
 * 
 * @param code_input Code input instance
 */
void code_input_free(CodeInput* code_input) {
    furi_assert(code_input);
    view_free(code_input->view);
    free(code_input);
}

/** 
 * @brief Get code input view
 * 
 * @param code_input code input instance
 * @return View instance that can be used for embedding
 */
View* code_input_get_view(CodeInput* code_input) {
    furi_assert(code_input);
    return code_input->view;
}

/** 
 * @brief Set code input callbacks
 * 
 * @param code_input code input instance
 * @param ok_callback input callback fn
 * @param fail_callback code match callback fn
 * @param callback_context callback context
 * @param buffer buffer 
 * @param buffer_length ptr to buffer length uint
 * @param ext_update  true to update buffer 
 */
void code_input_set_result_callback(
    CodeInput* code_input,
    CodeInputOkCallback ok_callback,
    CodeInputFailCallback fail_callback,
    void* callback_context,
    uint8_t* buffer,
    uint8_t* buffer_length,
    bool ext_update) {
    with_view_model(
        code_input->view, (CodeInputModel * model) {
            code_input_reset_model_input_data(model);
            model->ok_callback = ok_callback;
            model->fail_callback = fail_callback;
            model->callback_context = callback_context;

            model->ext_buffer = buffer;
            model->ext_buffer_length = buffer_length;
            model->state = (*buffer_length == 0) ? 1 : 0;
            model->ext_update = ext_update;

            return true;
        });
}

/**
 * @brief Set code input header text
 * 
 * @param code_input code input instance
 * @param text text to be shown
 */
void code_input_set_header_text(CodeInput* code_input, const char* text) {
    with_view_model(
        code_input->view, (CodeInputModel * model) {
            model->header = text;
            return true;
        });
}
