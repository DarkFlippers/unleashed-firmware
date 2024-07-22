#include "bt_test.h"

#include <gui/canvas.h>
#include <gui/elements.h>

#include <lib/toolbox/float_tools.h>
#include <m-array.h>
#include <furi.h>
#include <inttypes.h>
#include <stdint.h>

struct BtTestParam {
    const char* label;
    uint8_t current_value_index;
    FuriString* current_value_text;
    uint8_t values_count;
    BtTestParamChangeCallback change_callback;
    void* context;
};

ARRAY_DEF(BtTestParamArray, BtTestParam, M_POD_OPLIST);

struct BtTest {
    View* view;
    BtTestChangeStateCallback change_state_callback;
    BtTestBackCallback back_callback;
    void* context;
};

typedef struct {
    BtTestState state;
    BtTestParamArray_t params;
    uint8_t position;
    uint8_t window_position;
    const char* message;
    float rssi;
    uint32_t packets_num_rx;
    uint32_t packets_num_tx;
} BtTestModel;

#define BT_TEST_START_MESSAGE "Ok - Start"
#define BT_TEST_STOP_MESSAGE  "Ok - Stop"

static void bt_test_process_up(BtTest* bt_test);
static void bt_test_process_down(BtTest* bt_test);
static void bt_test_process_left(BtTest* bt_test);
static void bt_test_process_right(BtTest* bt_test);
static void bt_test_process_ok(BtTest* bt_test);
static void bt_test_process_back(BtTest* bt_test);

static void bt_test_draw_callback(Canvas* canvas, void* _model) {
    BtTestModel* model = _model;
    char info_str[32];

    const uint8_t param_height = 16;
    const uint8_t param_width = 123;

    canvas_clear(canvas);

    uint8_t position = 0;
    BtTestParamArray_it_t it;

    canvas_set_font(canvas, FontSecondary);
    for(BtTestParamArray_it(it, model->params); !BtTestParamArray_end_p(it);
        BtTestParamArray_next(it)) {
        uint8_t param_position = position - model->window_position;
        uint8_t params_on_screen = 3;
        uint8_t y_offset = 0;

        if(param_position < params_on_screen) {
            const BtTestParam* param = BtTestParamArray_cref(it);
            uint8_t param_y = y_offset + (param_position * param_height);
            uint8_t param_text_y = param_y + param_height - 4;

            if(position == model->position) {
                canvas_set_color(canvas, ColorBlack);
                elements_slightly_rounded_box(
                    canvas, 0, param_y + 1, param_width, param_height - 2);
                canvas_set_color(canvas, ColorWhite);
            } else {
                canvas_set_color(canvas, ColorBlack);
            }

            canvas_draw_str(canvas, 6, param_text_y, param->label);

            if(param->current_value_index > 0) {
                canvas_draw_str(canvas, 50, param_text_y, "<");
            }

            canvas_draw_str(
                canvas, 61, param_text_y, furi_string_get_cstr(param->current_value_text));

            if(param->current_value_index < (param->values_count - 1)) {
                canvas_draw_str(canvas, 113, param_text_y, ">");
            }
        }

        position++;
    }

    elements_scrollbar(canvas, model->position, BtTestParamArray_size(model->params));
    canvas_draw_str(canvas, 6, 60, model->message);
    if(model->state == BtTestStateStarted) {
        if(!float_is_equal(model->rssi, 0.0f)) {
            snprintf(info_str, sizeof(info_str), "RSSI:%3.1f dB", (double)model->rssi);
            canvas_draw_str_aligned(canvas, 124, 60, AlignRight, AlignBottom, info_str);
        }
    } else if(model->state == BtTestStateStopped) {
        if(model->packets_num_rx) {
            snprintf(info_str, sizeof(info_str), "%" PRIu32 " pack rcv", model->packets_num_rx);
            canvas_draw_str_aligned(canvas, 124, 60, AlignRight, AlignBottom, info_str);
        } else if(model->packets_num_tx) {
            snprintf(info_str, sizeof(info_str), "%" PRIu32 " pack sent", model->packets_num_tx);
            canvas_draw_str_aligned(canvas, 124, 60, AlignRight, AlignBottom, info_str);
        }
    }
}

static bool bt_test_input_callback(InputEvent* event, void* context) {
    BtTest* bt_test = context;
    furi_assert(bt_test);
    bool consumed = false;

    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyUp:
            consumed = true;
            bt_test_process_up(bt_test);
            break;
        case InputKeyDown:
            consumed = true;
            bt_test_process_down(bt_test);
            break;
        case InputKeyLeft:
            consumed = true;
            bt_test_process_left(bt_test);
            break;
        case InputKeyRight:
            consumed = true;
            bt_test_process_right(bt_test);
            break;
        case InputKeyOk:
            consumed = true;
            bt_test_process_ok(bt_test);
            break;
        case InputKeyBack:
            consumed = false;
            bt_test_process_back(bt_test);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void bt_test_process_up(BtTest* bt_test) {
    with_view_model( // -V658
        bt_test->view,
        BtTestModel * model,
        {
            uint8_t params_on_screen = 3;
            if(model->position > 0) {
                model->position--;
                if(((model->position - model->window_position) < 1) &&
                   model->window_position > 0) {
                    model->window_position--;
                }
            } else {
                model->position = BtTestParamArray_size(model->params) - 1;
                if(model->position > (params_on_screen - 1)) {
                    model->window_position = model->position - (params_on_screen - 1);
                }
            }
        },
        true);
}

void bt_test_process_down(BtTest* bt_test) {
    with_view_model(
        bt_test->view,
        BtTestModel * model,
        {
            uint8_t params_on_screen = 3;
            if(model->position < (BtTestParamArray_size(model->params) - 1)) {
                model->position++;
                if((model->position - model->window_position) > (params_on_screen - 2) &&
                   model->window_position <
                       (BtTestParamArray_size(model->params) - params_on_screen)) {
                    model->window_position++;
                }
            } else {
                model->position = 0;
                model->window_position = 0;
            }
        },
        true);
}

BtTestParam* bt_test_get_selected_param(BtTestModel* model) {
    BtTestParam* param = NULL;

    BtTestParamArray_it_t it;
    uint8_t position = 0;
    for(BtTestParamArray_it(it, model->params); !BtTestParamArray_end_p(it);
        BtTestParamArray_next(it)) {
        if(position == model->position) {
            break;
        }
        position++;
    }

    param = BtTestParamArray_ref(it);

    furi_assert(param);
    return param;
}

void bt_test_process_left(BtTest* bt_test) {
    BtTestParam* param;
    with_view_model(
        bt_test->view,
        BtTestModel * model,
        {
            param = bt_test_get_selected_param(model);
            if(param->current_value_index > 0) {
                param->current_value_index--;
                if(param->change_callback) {
                    model->state = BtTestStateStopped;
                    model->message = BT_TEST_START_MESSAGE;
                    model->rssi = 0.0f;
                    model->packets_num_rx = 0;
                    model->packets_num_tx = 0;
                }
            }
        },
        true);
    if(param->change_callback) {
        param->change_callback(param);
    }
}

void bt_test_process_right(BtTest* bt_test) {
    BtTestParam* param;
    with_view_model(
        bt_test->view,
        BtTestModel * model,
        {
            param = bt_test_get_selected_param(model);
            if(param->current_value_index < (param->values_count - 1)) {
                param->current_value_index++;
                if(param->change_callback) {
                    model->state = BtTestStateStopped;
                    model->message = BT_TEST_START_MESSAGE;
                    model->rssi = 0.0f;
                    model->packets_num_rx = 0;
                    model->packets_num_tx = 0;
                }
            }
        },
        true);
    if(param->change_callback) {
        param->change_callback(param);
    }
}

void bt_test_process_ok(BtTest* bt_test) {
    BtTestState state;
    with_view_model(
        bt_test->view,
        BtTestModel * model,
        {
            if(model->state == BtTestStateStarted) {
                model->state = BtTestStateStopped;
                model->message = BT_TEST_START_MESSAGE;
                model->rssi = 0.0f;
                model->packets_num_rx = 0;
                model->packets_num_tx = 0;
            } else if(model->state == BtTestStateStopped) {
                model->state = BtTestStateStarted;
                model->message = BT_TEST_STOP_MESSAGE;
            }
            state = model->state;
        },
        true);
    if(bt_test->change_state_callback) {
        bt_test->change_state_callback(state, bt_test->context);
    }
}

void bt_test_process_back(BtTest* bt_test) {
    with_view_model(
        bt_test->view,
        BtTestModel * model,
        {
            model->state = BtTestStateStopped;
            model->rssi = 0.0f;
            model->packets_num_rx = 0;
            model->packets_num_tx = 0;
        },
        false);
    if(bt_test->back_callback) {
        bt_test->back_callback(bt_test->context);
    }
}

BtTest* bt_test_alloc(void) {
    BtTest* bt_test = malloc(sizeof(BtTest));
    bt_test->view = view_alloc();
    view_set_context(bt_test->view, bt_test);
    view_allocate_model(bt_test->view, ViewModelTypeLocking, sizeof(BtTestModel));
    view_set_draw_callback(bt_test->view, bt_test_draw_callback);
    view_set_input_callback(bt_test->view, bt_test_input_callback);

    with_view_model(
        bt_test->view,
        BtTestModel * model,
        {
            model->state = BtTestStateStopped;
            model->message = "Ok - Start";
            BtTestParamArray_init(model->params);
            model->position = 0;
            model->window_position = 0;
            model->rssi = 0.0f;
            model->packets_num_tx = 0;
            model->packets_num_rx = 0;
        },
        true);

    return bt_test;
}

void bt_test_free(BtTest* bt_test) {
    furi_assert(bt_test);

    with_view_model(
        bt_test->view,
        BtTestModel * model,
        {
            BtTestParamArray_it_t it;
            for(BtTestParamArray_it(it, model->params); !BtTestParamArray_end_p(it);
                BtTestParamArray_next(it)) {
                furi_string_free(BtTestParamArray_ref(it)->current_value_text);
            }
            BtTestParamArray_clear(model->params);
        },
        false);
    view_free(bt_test->view);
    free(bt_test);
}

View* bt_test_get_view(BtTest* bt_test) {
    furi_assert(bt_test);
    return bt_test->view;
}

BtTestParam* bt_test_param_add(
    BtTest* bt_test,
    const char* label,
    uint8_t values_count,
    BtTestParamChangeCallback change_callback,
    void* context) {
    BtTestParam* param = NULL;
    furi_assert(label);
    furi_assert(bt_test);

    with_view_model(
        bt_test->view,
        BtTestModel * model,
        {
            param = BtTestParamArray_push_new(model->params);
            param->label = label;
            param->values_count = values_count;
            param->change_callback = change_callback;
            param->context = context;
            param->current_value_index = 0;
            param->current_value_text = furi_string_alloc();
        },
        true);

    return param;
}

void bt_test_set_rssi(BtTest* bt_test, float rssi) {
    furi_assert(bt_test);
    with_view_model(bt_test->view, BtTestModel * model, { model->rssi = rssi; }, true);
}

void bt_test_set_packets_tx(BtTest* bt_test, uint32_t packets_num) {
    furi_assert(bt_test);
    with_view_model(
        bt_test->view, BtTestModel * model, { model->packets_num_tx = packets_num; }, true);
}

void bt_test_set_packets_rx(BtTest* bt_test, uint32_t packets_num) {
    furi_assert(bt_test);
    with_view_model(
        bt_test->view, BtTestModel * model, { model->packets_num_rx = packets_num; }, true);
}

void bt_test_set_change_state_callback(BtTest* bt_test, BtTestChangeStateCallback callback) {
    furi_assert(bt_test);
    furi_assert(callback);
    bt_test->change_state_callback = callback;
}

void bt_test_set_back_callback(BtTest* bt_test, BtTestBackCallback callback) {
    furi_assert(bt_test);
    furi_assert(callback);
    bt_test->back_callback = callback;
}

void bt_test_set_context(BtTest* bt_test, void* context) {
    furi_assert(bt_test);
    bt_test->context = context;
}

void bt_test_set_current_value_index(BtTestParam* param, uint8_t current_value_index) {
    param->current_value_index = current_value_index;
}

void bt_test_set_current_value_text(BtTestParam* param, const char* current_value_text) {
    furi_string_set(param->current_value_text, current_value_text);
}

uint8_t bt_test_get_current_value_index(BtTestParam* param) {
    return param->current_value_index;
}

void* bt_test_get_context(BtTestParam* param) {
    return param->context;
}
