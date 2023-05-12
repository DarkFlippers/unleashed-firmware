#include "bad_bt_view.h"
#include "../helpers/ducky_script.h"
#include "../bad_bt_app.h"
#include <toolbox/path.h>
#include <gui/elements.h>
#include <assets_icons.h>

#define MAX_NAME_LEN 64

typedef struct {
    char file_name[MAX_NAME_LEN];
    char layout[MAX_NAME_LEN];
    BadBtState state;
    uint8_t anim_frame;
} BadBtModel;

static void bad_bt_draw_callback(Canvas* canvas, void* _model) {
    BadBtModel* model = _model;

    FuriString* disp_str;
    disp_str = furi_string_alloc_set("(BT) ");
    furi_string_cat_str(disp_str, model->file_name);
    elements_string_fit_width(canvas, disp_str, 128 - 2);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 8, furi_string_get_cstr(disp_str));

    if(strlen(model->layout) == 0) {
        furi_string_set(disp_str, "(default)");
    } else {
        furi_string_reset(disp_str);
        furi_string_push_back(disp_str, '(');
        for(size_t i = 0; i < strlen(model->layout); i++)
            furi_string_push_back(disp_str, model->layout[i]);
        furi_string_push_back(disp_str, ')');
    }
    if(model->state.pin) {
        furi_string_cat_printf(disp_str, "  PIN: %ld", model->state.pin);
    }
    elements_string_fit_width(canvas, disp_str, 128 - 2);
    canvas_draw_str(
        canvas, 2, 8 + canvas_current_font_height(canvas), furi_string_get_cstr(disp_str));

    furi_string_reset(disp_str);

    if((model->state.state == BadBtStateIdle) || (model->state.state == BadBtStateDone) ||
       (model->state.state == BadBtStateNotConnected)) {
        elements_button_center(canvas, "Run");
        elements_button_left(canvas, "Config");
    } else if((model->state.state == BadBtStateRunning) || (model->state.state == BadBtStateDelay)) {
        elements_button_center(canvas, "Stop");
    } else if(model->state.state == BadBtStateWaitForBtn) {
        elements_button_center(canvas, "Press to continue");
    } else if(model->state.state == BadBtStateWillRun) {
        elements_button_center(canvas, "Cancel");
    }

    if(model->state.state == BadBtStateNotConnected) {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "Connect to");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "a device");
    } else if(model->state.state == BadBtStateWillRun) {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "Will run");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "on connect");
    } else if(model->state.state == BadBtStateFileError) {
        canvas_draw_icon(canvas, 4, 26, &I_Error_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "File");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "ERROR");
    } else if(model->state.state == BadBtStateScriptError) {
        canvas_draw_icon(canvas, 4, 26, &I_Error_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 33, AlignRight, AlignBottom, "ERROR:");
        canvas_set_font(canvas, FontSecondary);
        furi_string_printf(disp_str, "line %u", model->state.error_line);
        canvas_draw_str_aligned(
            canvas, 127, 46, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        furi_string_set_str(disp_str, model->state.error);
        elements_string_fit_width(canvas, disp_str, canvas_width(canvas));
        canvas_draw_str_aligned(
            canvas, 127, 56, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
    } else if(model->state.state == BadBtStateIdle) {
        canvas_draw_icon(canvas, 4, 26, &I_Smile_18x18);
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 114, 40, AlignRight, AlignBottom, "0");
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(model->state.state == BadBtStateRunning) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviSmile1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviSmile2_18x21);
        }
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(
            disp_str, "%u", ((model->state.line_cur - 1) * 100) / model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 114, 40, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(model->state.state == BadBtStateDone) {
        canvas_draw_icon(canvas, 4, 23, &I_EviSmile1_18x21);
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 114, 40, AlignRight, AlignBottom, "100");
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(model->state.state == BadBtStateDelay) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting2_18x21);
        }
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(
            disp_str, "%u", ((model->state.line_cur - 1) * 100) / model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 114, 40, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
        canvas_set_font(canvas, FontSecondary);
        furi_string_printf(disp_str, "delay %lus", model->state.delay_remain);
        canvas_draw_str_aligned(
            canvas, 127, 50, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
    } else {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
    }

    furi_string_free(disp_str);
}

static bool bad_bt_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    BadBt* bad_bt = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if((event->key == InputKeyLeft) || (event->key == InputKeyOk)) {
            consumed = true;
            furi_assert(bad_bt->callback);
            bad_bt->callback(event->key, bad_bt->context);
        }
    }

    return consumed;
}

BadBt* bad_bt_alloc() {
    BadBt* bad_bt = malloc(sizeof(BadBt));

    bad_bt->view = view_alloc();
    view_allocate_model(bad_bt->view, ViewModelTypeLocking, sizeof(BadBtModel));
    view_set_context(bad_bt->view, bad_bt);
    view_set_draw_callback(bad_bt->view, bad_bt_draw_callback);
    view_set_input_callback(bad_bt->view, bad_bt_input_callback);

    return bad_bt;
}

void bad_bt_free(BadBt* bad_bt) {
    furi_assert(bad_bt);
    view_free(bad_bt->view);
    free(bad_bt);
}

View* bad_bt_get_view(BadBt* bad_bt) {
    furi_assert(bad_bt);
    return bad_bt->view;
}

void bad_bt_set_button_callback(BadBt* bad_bt, BadBtButtonCallback callback, void* context) {
    furi_assert(bad_bt);
    furi_assert(callback);
    with_view_model(
        bad_bt->view,
        BadBtModel * model,
        {
            UNUSED(model);
            bad_bt->callback = callback;
            bad_bt->context = context;
        },
        true);
}

void bad_bt_set_file_name(BadBt* bad_bt, const char* name) {
    furi_assert(name);
    with_view_model(
        bad_bt->view, BadBtModel * model, { strlcpy(model->file_name, name, MAX_NAME_LEN); }, true);
}

void bad_bt_set_layout(BadBt* bad_bt, const char* layout) {
    furi_assert(layout);
    with_view_model(
        bad_bt->view, BadBtModel * model, { strlcpy(model->layout, layout, MAX_NAME_LEN); }, true);
}

void bad_bt_set_state(BadBt* bad_bt, BadBtState* st) {
    furi_assert(st);
    uint32_t pin = 0;
    if(bad_bt->context != NULL) {
        BadBtApp* app = bad_bt->context;
        if(app->bt != NULL) {
            pin = app->bt->pin;
        }
    }
    st->pin = pin;
    with_view_model(
        bad_bt->view,
        BadBtModel * model,
        {
            memcpy(&(model->state), st, sizeof(BadBtState));
            model->anim_frame ^= 1;
        },
        true);
}

bool bad_bt_is_idle_state(BadBt* bad_bt) {
    bool is_idle = false;
    with_view_model(
        bad_bt->view,
        BadBtModel * model,
        {
            if((model->state.state == BadBtStateIdle) || (model->state.state == BadBtStateDone) ||
               (model->state.state == BadBtStateNotConnected)) {
                is_idle = true;
            }
        },
        false);
    return is_idle;
}
