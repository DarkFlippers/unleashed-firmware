#include "bad_ble_view.h"
#include "../helpers/ducky_script.h"
#include <toolbox/path.h>
#include <gui/elements.h>
#include <assets_icons.h>
#include "bad_ble_icons.h"

#define MAX_NAME_LEN 64

struct BadBle {
    View* view;
    BadBleButtonCallback callback;
    void* context;
};

typedef struct {
    char file_name[MAX_NAME_LEN];
    char layout[MAX_NAME_LEN];
    BadBleState state;
    bool pause_wait;
    uint8_t anim_frame;
} BadBleModel;

static void bad_ble_draw_callback(Canvas* canvas, void* _model) {
    BadBleModel* model = _model;

    FuriString* disp_str;
    disp_str = furi_string_alloc_set(model->file_name);
    elements_string_fit_width(canvas, disp_str, 128 - 2);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 8, furi_string_get_cstr(disp_str));

    if(strlen(model->layout) == 0) {
        furi_string_set(disp_str, "(default)");
    } else {
        furi_string_printf(disp_str, "(%s)", model->layout);
    }
    elements_string_fit_width(canvas, disp_str, 128 - 2);
    canvas_draw_str(
        canvas, 2, 8 + canvas_current_font_height(canvas), furi_string_get_cstr(disp_str));

    furi_string_reset(disp_str);

    canvas_draw_icon(canvas, 22, 24, &I_Bad_BLE_48x22);

    BadBleWorkerState state = model->state.state;

    if((state == BadBleStateIdle) || (state == BadBleStateDone) ||
       (state == BadBleStateNotConnected)) {
        elements_button_center(canvas, "Run");
        elements_button_left(canvas, "Config");
    } else if((state == BadBleStateRunning) || (state == BadBleStateDelay)) {
        elements_button_center(canvas, "Stop");
        if(!model->pause_wait) {
            elements_button_right(canvas, "Pause");
        }
    } else if(state == BadBleStatePaused) {
        elements_button_center(canvas, "End");
        elements_button_right(canvas, "Resume");
    } else if(state == BadBleStateWaitForBtn) {
        elements_button_center(canvas, "Press to continue");
    } else if(state == BadBleStateWillRun) {
        elements_button_center(canvas, "Cancel");
    }

    if(state == BadBleStateNotConnected) {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "Connect");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "to device");
    } else if(state == BadBleStateWillRun) {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "Will run");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "on connect");
    } else if(state == BadBleStateFileError) {
        canvas_draw_icon(canvas, 4, 26, &I_Error_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "File");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "ERROR");
    } else if(state == BadBleStateScriptError) {
        canvas_draw_icon(canvas, 4, 26, &I_Error_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 33, AlignRight, AlignBottom, "ERROR:");
        canvas_set_font(canvas, FontSecondary);
        furi_string_printf(disp_str, "line %zu", model->state.error_line);
        canvas_draw_str_aligned(
            canvas, 127, 46, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);

        furi_string_set_str(disp_str, model->state.error);
        elements_string_fit_width(canvas, disp_str, canvas_width(canvas));
        canvas_draw_str_aligned(
            canvas, 127, 56, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
    } else if(state == BadBleStateIdle) {
        canvas_draw_icon(canvas, 4, 26, &I_Smile_18x18);
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 114, 40, AlignRight, AlignBottom, "0");
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(state == BadBleStateRunning) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviSmile1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviSmile2_18x21);
        }
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(
            disp_str, "%zu", ((model->state.line_cur - 1) * 100) / model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 114, 40, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(state == BadBleStateDone) {
        canvas_draw_icon(canvas, 4, 23, &I_EviSmile1_18x21);
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 114, 40, AlignRight, AlignBottom, "100");
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(state == BadBleStateDelay) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting2_18x21);
        }
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(
            disp_str, "%zu", ((model->state.line_cur - 1) * 100) / model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 114, 40, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
        canvas_set_font(canvas, FontSecondary);
        furi_string_printf(disp_str, "delay %lus", model->state.delay_remain);
        canvas_draw_str_aligned(
            canvas, 127, 50, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
    } else if((state == BadBleStatePaused) || (state == BadBleStateWaitForBtn)) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting2_18x21);
        }
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(
            disp_str, "%zu", ((model->state.line_cur - 1) * 100) / model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 114, 40, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 127, 50, AlignRight, AlignBottom, "Paused");
        furi_string_reset(disp_str);
    } else {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
    }

    furi_string_free(disp_str);
}

static bool bad_ble_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    BadBle* bad_ble = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyLeft) {
            consumed = true;
            furi_assert(bad_ble->callback);
            bad_ble->callback(event->key, bad_ble->context);
        } else if(event->key == InputKeyOk) {
            with_view_model(
                bad_ble->view, BadBleModel * model, { model->pause_wait = false; }, true);
            consumed = true;
            furi_assert(bad_ble->callback);
            bad_ble->callback(event->key, bad_ble->context);
        } else if(event->key == InputKeyRight) {
            with_view_model(
                bad_ble->view,
                BadBleModel * model,
                {
                    if((model->state.state == BadBleStateRunning) ||
                       (model->state.state == BadBleStateDelay)) {
                        model->pause_wait = true;
                    }
                },
                true);
            consumed = true;
            furi_assert(bad_ble->callback);
            bad_ble->callback(event->key, bad_ble->context);
        }
    }

    return consumed;
}

BadBle* bad_ble_view_alloc(void) {
    BadBle* bad_ble = malloc(sizeof(BadBle));

    bad_ble->view = view_alloc();
    view_allocate_model(bad_ble->view, ViewModelTypeLocking, sizeof(BadBleModel));
    view_set_context(bad_ble->view, bad_ble);
    view_set_draw_callback(bad_ble->view, bad_ble_draw_callback);
    view_set_input_callback(bad_ble->view, bad_ble_input_callback);

    return bad_ble;
}

void bad_ble_view_free(BadBle* bad_ble) {
    furi_assert(bad_ble);
    view_free(bad_ble->view);
    free(bad_ble);
}

View* bad_ble_view_get_view(BadBle* bad_ble) {
    furi_assert(bad_ble);
    return bad_ble->view;
}

void bad_ble_view_set_button_callback(
    BadBle* bad_ble,
    BadBleButtonCallback callback,
    void* context) {
    furi_assert(bad_ble);
    furi_assert(callback);
    with_view_model(
        bad_ble->view,
        BadBleModel * model,
        {
            UNUSED(model);
            bad_ble->callback = callback;
            bad_ble->context = context;
        },
        true);
}

void bad_ble_view_set_file_name(BadBle* bad_ble, const char* name) {
    furi_assert(name);
    with_view_model(
        bad_ble->view,
        BadBleModel * model,
        { strlcpy(model->file_name, name, MAX_NAME_LEN); },
        true);
}

void bad_ble_view_set_layout(BadBle* bad_ble, const char* layout) {
    furi_assert(layout);
    with_view_model(
        bad_ble->view,
        BadBleModel * model,
        { strlcpy(model->layout, layout, MAX_NAME_LEN); },
        true);
}

void bad_ble_view_set_state(BadBle* bad_ble, BadBleState* st) {
    furi_assert(st);
    with_view_model(
        bad_ble->view,
        BadBleModel * model,
        {
            memcpy(&(model->state), st, sizeof(BadBleState));
            model->anim_frame ^= 1;
            if(model->state.state == BadBleStatePaused) {
                model->pause_wait = false;
            }
        },
        true);
}

bool bad_ble_view_is_idle_state(BadBle* bad_ble) {
    bool is_idle = false;
    with_view_model(
        bad_ble->view,
        BadBleModel * model,
        {
            if((model->state.state == BadBleStateIdle) ||
               (model->state.state == BadBleStateDone) ||
               (model->state.state == BadBleStateNotConnected)) {
                is_idle = true;
            }
        },
        false);
    return is_idle;
}
