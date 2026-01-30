#include "bad_duck3_view.h"
#include <toolbox/path.h>
#include <gui/elements.h>
#include <assets_icons.h>
#include <bt/bt_service/bt_i.h>

#define MAX_NAME_LEN 64

struct BadDuck3View {
    View* view;
    BadDuck3ButtonCallback callback;
    void* context;
};

typedef struct {
    char file_name[MAX_NAME_LEN];
    char layout[MAX_NAME_LEN];
    Duck3State state;
    bool pause_wait;
    uint8_t anim_frame;
    Duck3HidInterface interface;
    Bt* bt;
} BadDuck3ViewModel;

static void bad_duck3_draw_callback(Canvas* canvas, void* _model) {
    BadDuck3ViewModel* model = _model;

    FuriString* disp_str = furi_string_alloc_set(model->file_name);
    elements_string_fit_width(canvas, disp_str, 128 - 2);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 8, furi_string_get_cstr(disp_str));

    if(strlen(model->layout) == 0) {
        furi_string_set(disp_str, "(default)");
    } else {
        furi_string_printf(disp_str, "(%s)", model->layout);
    }
    if(model->interface == Duck3HidInterfaceBle && model->bt->pin_code) {
        furi_string_cat_printf(disp_str, "  PIN: %ld", model->bt->pin_code);
    } else {
        uint32_t e = model->state.elapsed;
        furi_string_cat_printf(disp_str, "  %02lu:%02lu.%ld", e / 60 / 1000, e / 1000 % 60, (e % 1000) / 100);
    }
    elements_string_fit_width(canvas, disp_str, 128 - 2);
    canvas_draw_str(
        canvas, 2, 8 + canvas_current_font_height(canvas), furi_string_get_cstr(disp_str));

    furi_string_reset(disp_str);

    // Draw interface icon
    if(model->interface == Duck3HidInterfaceBle) {
        canvas_draw_icon(canvas, 22, 24, &I_Bad_BLE_48x22);
    } else {
        canvas_draw_icon(canvas, 22, 24, &I_UsbTree_48x22);
    }

    Duck3WorkerState state = model->state.state;

    // Draw buttons based on state
    if((state == Duck3StateIdle) || (state == Duck3StateDone) ||
       (state == Duck3StateNotConnected)) {
        elements_button_center(canvas, "Run");
        elements_button_left(canvas, "Config");
        elements_button_right(canvas, model->interface == Duck3HidInterfaceBle ? "USB" : "BLE");
    } else if((state == Duck3StateRunning) || (state == Duck3StateDelay)) {
        elements_button_center(canvas, "Stop");
        if(!model->pause_wait) {
            elements_button_right(canvas, "Pause");
        }
    } else if(state == Duck3StatePaused) {
        elements_button_center(canvas, "End");
        elements_button_right(canvas, "Resume");
    } else if(state == Duck3StateWaitForBtn) {
        elements_button_center(canvas, "Press to continue");
    } else if(state == Duck3StateWillRun) {
        elements_button_center(canvas, "Cancel");
    }

    // Draw state-specific info
    if(state == Duck3StateNotConnected) {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "Connect");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "to device");
    } else if(state == Duck3StateWillRun) {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "Will run");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "on connect");
    } else if(state == Duck3StateFileError) {
        canvas_draw_icon(canvas, 4, 26, &I_Error_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "File");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "ERROR");
    } else if(state == Duck3StateScriptError) {
        canvas_draw_icon(canvas, 4, 26, &I_Error_18x18);
        furi_string_printf(disp_str, "line %lu", model->state.error_line);
        canvas_draw_str_aligned(
            canvas, 127, 46, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_set_str(disp_str, model->state.error);
        elements_string_fit_width(canvas, disp_str, canvas_width(canvas));
        canvas_draw_str_aligned(
            canvas, 127, 56, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 33, AlignRight, AlignBottom, "ERROR:");
    } else if(state == Duck3StateIdle) {
        canvas_draw_icon(canvas, 4, 26, &I_Smile_18x18);
        furi_string_printf(disp_str, "0/%lu", model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 124, 47, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 112, 37, AlignRight, AlignBottom, "0");
        canvas_draw_icon(canvas, 115, 23, &I_Percent_10x14);
    } else if(state == Duck3StateRunning) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviSmile1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviSmile2_18x21);
        }
        furi_string_printf(disp_str, "%lu/%lu", model->state.line_cur, model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 124, 47, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        canvas_set_font(canvas, FontBigNumbers);
        size_t pct = model->state.line_nb > 0 ?
            ((model->state.line_cur) * 100) / model->state.line_nb : 0;
        furi_string_printf(disp_str, "%lu", (long unsigned int)pct);
        canvas_draw_str_aligned(
            canvas, 112, 37, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        canvas_draw_icon(canvas, 115, 23, &I_Percent_10x14);
    } else if(state == Duck3StateDone) {
        canvas_draw_icon(canvas, 4, 23, &I_EviSmile1_18x21);
        furi_string_printf(disp_str, "%lu/%lu", model->state.line_nb, model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 124, 47, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 112, 37, AlignRight, AlignBottom, "100");
        canvas_draw_icon(canvas, 115, 23, &I_Percent_10x14);
    } else if(state == Duck3StateDelay) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting2_18x21);
        }
        uint32_t delay = model->state.delay_remain / 10;
        if(delay) {
            furi_string_printf(disp_str, "Delay %lus", delay);
            canvas_draw_str_aligned(
                canvas, 4, 61, AlignLeft, AlignBottom, furi_string_get_cstr(disp_str));
        }
        furi_string_printf(disp_str, "%lu/%lu", model->state.line_cur, model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 124, 47, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        canvas_set_font(canvas, FontBigNumbers);
        size_t pct = model->state.line_nb > 0 ?
            ((model->state.line_cur) * 100) / model->state.line_nb : 0;
        furi_string_printf(disp_str, "%lu", (long unsigned int)pct);
        canvas_draw_str_aligned(
            canvas, 112, 37, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        canvas_draw_icon(canvas, 115, 23, &I_Percent_10x14);
    } else if((state == Duck3StatePaused) || (state == Duck3StateWaitForBtn)) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting2_18x21);
        }
        if(state != Duck3StateWaitForBtn) {
            canvas_draw_str_aligned(canvas, 4, 61, AlignLeft, AlignBottom, "Paused");
        }
        furi_string_printf(disp_str, "%lu/%lu", model->state.line_cur, model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 124, 47, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        canvas_set_font(canvas, FontBigNumbers);
        size_t pct = model->state.line_nb > 0 ?
            ((model->state.line_cur) * 100) / model->state.line_nb : 0;
        furi_string_printf(disp_str, "%lu", (long unsigned int)pct);
        canvas_draw_str_aligned(
            canvas, 112, 37, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        canvas_draw_icon(canvas, 115, 23, &I_Percent_10x14);
    } else {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
    }

    furi_string_free(disp_str);
}

static bool bad_duck3_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    BadDuck3View* view = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyLeft) {
            consumed = true;
            furi_assert(view->callback);
            view->callback(event->key, view->context);
        } else if(event->key == InputKeyOk) {
            with_view_model(
                view->view, BadDuck3ViewModel * model, { model->pause_wait = false; }, true);
            consumed = true;
            furi_assert(view->callback);
            view->callback(event->key, view->context);
        } else if(event->key == InputKeyRight) {
            with_view_model(
                view->view,
                BadDuck3ViewModel * model,
                {
                    if((model->state.state == Duck3StateRunning) ||
                       (model->state.state == Duck3StateDelay)) {
                        model->pause_wait = true;
                    }
                },
                true);
            consumed = true;
            furi_assert(view->callback);
            view->callback(event->key, view->context);
        }
    }

    return consumed;
}

BadDuck3View* bad_duck3_view_alloc(void) {
    BadDuck3View* view = malloc(sizeof(BadDuck3View));

    view->view = view_alloc();
    view_allocate_model(view->view, ViewModelTypeLocking, sizeof(BadDuck3ViewModel));
    view_set_context(view->view, view);
    view_set_draw_callback(view->view, bad_duck3_draw_callback);
    view_set_input_callback(view->view, bad_duck3_input_callback);

    with_view_model(
        view->view, BadDuck3ViewModel * model, { model->bt = furi_record_open(RECORD_BT); }, true);

    return view;
}

void bad_duck3_view_free(BadDuck3View* view) {
    furi_assert(view);
    furi_record_close(RECORD_BT);
    view_free(view->view);
    free(view);
}

View* bad_duck3_view_get_view(BadDuck3View* view) {
    furi_assert(view);
    return view->view;
}

void bad_duck3_view_set_button_callback(
    BadDuck3View* view,
    BadDuck3ButtonCallback callback,
    void* context) {
    furi_assert(view);
    furi_assert(callback);
    with_view_model(
        view->view,
        BadDuck3ViewModel * model,
        {
            UNUSED(model);
            view->callback = callback;
            view->context = context;
        },
        true);
}

void bad_duck3_view_set_file_name(BadDuck3View* view, const char* name) {
    furi_assert(name);
    with_view_model(
        view->view,
        BadDuck3ViewModel * model,
        { strlcpy(model->file_name, name, MAX_NAME_LEN); },
        true);
}

void bad_duck3_view_set_layout(BadDuck3View* view, const char* layout) {
    furi_assert(layout);
    with_view_model(
        view->view,
        BadDuck3ViewModel * model,
        { strlcpy(model->layout, layout, MAX_NAME_LEN); },
        true);
}

void bad_duck3_view_set_state(BadDuck3View* view, Duck3State* st) {
    furi_assert(st);
    with_view_model(
        view->view,
        BadDuck3ViewModel * model,
        {
            memcpy(&(model->state), st, sizeof(Duck3State));
            model->anim_frame ^= 1;
            if(model->state.state == Duck3StatePaused) {
                model->pause_wait = false;
            }
        },
        true);
}

void bad_duck3_view_set_interface(BadDuck3View* view, Duck3HidInterface interface) {
    with_view_model(view->view, BadDuck3ViewModel * model, { model->interface = interface; }, true);
}

bool bad_duck3_view_is_idle_state(BadDuck3View* view) {
    bool is_idle = false;
    with_view_model(
        view->view,
        BadDuck3ViewModel * model,
        {
            if((model->state.state == Duck3StateIdle) ||
               (model->state.state == Duck3StateDone) ||
               (model->state.state == Duck3StateNotConnected)) {
                is_idle = true;
            }
        },
        false);
    return is_idle;
}
