#include "hid_mouse_jiggler.h"
#include <gui/elements.h>
#include "../hid.h"

#include "hid_icons.h"

#define TAG "HidMouseJiggler"

struct HidMouseJiggler {
    View* view;
    Hid* hid;
    FuriTimer* timer;
};

typedef struct {
    bool connected;
    bool running;
    int interval_idx;
    uint8_t counter;
} HidMouseJigglerModel;

const int intervals[6] = {500, 2000, 5000, 10000, 30000, 60000};

static void hid_mouse_jiggler_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidMouseJigglerModel* model = context;

    // Header
#ifdef HID_TRANSPORT_BLE
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }
#endif

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 27, 2, AlignLeft, AlignTop, "Mouse Jiggler");

    // Timeout
    elements_multiline_text(canvas, AlignLeft, 26, "Interval (ms):");
    canvas_set_font(canvas, FontSecondary);
    if(model->interval_idx != 0) canvas_draw_icon(canvas, 74, 19, &I_ButtonLeft_4x7);
    if(model->interval_idx != (int)COUNT_OF(intervals) - 1)
        canvas_draw_icon(canvas, 80, 19, &I_ButtonRight_4x7);
    FuriString* interval_str = furi_string_alloc_printf("%d", intervals[model->interval_idx]);
    elements_multiline_text(canvas, 91, 26, furi_string_get_cstr(interval_str));
    furi_string_free(interval_str);

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text(canvas, AlignLeft, 40, "Press Start\nto jiggle");
    canvas_set_font(canvas, FontSecondary);

    // Ok
    canvas_draw_icon(canvas, 63, 30, &I_Space_65x18);
    if(model->running) {
        elements_slightly_rounded_box(canvas, 66, 32, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 74, 34, &I_Ok_btn_9x9);
    if(model->running) {
        elements_multiline_text_aligned(canvas, 91, 41, AlignLeft, AlignBottom, "Stop");
    } else {
        elements_multiline_text_aligned(canvas, 91, 41, AlignLeft, AlignBottom, "Start");
    }
    canvas_set_color(canvas, ColorBlack);

    // Back
    canvas_draw_icon(canvas, 74, 54, &I_Pin_back_arrow_10x8);
    elements_multiline_text_aligned(canvas, 91, 62, AlignLeft, AlignBottom, "Quit");
}

static void hid_mouse_jiggler_timer_callback(void* context) {
    furi_assert(context);
    HidMouseJiggler* hid_mouse_jiggler = context;
    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerModel * model,
        {
            if(model->running) {
                model->counter++;
                hid_hal_mouse_move(
                    hid_mouse_jiggler->hid,
                    (model->counter % 2 == 0) ? MOUSE_MOVE_SHORT : -MOUSE_MOVE_SHORT,
                    0);
            }
        },
        false);
}

static void hid_mouse_jiggler_exit_callback(void* context) {
    furi_assert(context);
    HidMouseJiggler* hid_mouse_jiggler = context;
    furi_timer_stop(hid_mouse_jiggler->timer);
}

static bool hid_mouse_jiggler_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidMouseJiggler* hid_mouse_jiggler = context;

    bool consumed = false;

    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerModel * model,
        {
            if(event->type == InputTypePress && event->key == InputKeyOk) {
                model->running = !model->running;
                if(model->running) {
                    furi_timer_stop(hid_mouse_jiggler->timer);
                    furi_timer_start(hid_mouse_jiggler->timer, intervals[model->interval_idx]);
                };
                consumed = true;
            }
            if(event->type == InputTypePress && event->key == InputKeyRight && !model->running &&
               model->interval_idx < (int)COUNT_OF(intervals) - 1) {
                model->interval_idx++;
                consumed = true;
            }
            if(event->type == InputTypePress && event->key == InputKeyLeft && !model->running &&
               model->interval_idx > 0) {
                model->interval_idx--;
                consumed = true;
            }
        },
        true);

    return consumed;
}

HidMouseJiggler* hid_mouse_jiggler_alloc(Hid* hid) {
    HidMouseJiggler* hid_mouse_jiggler = malloc(sizeof(HidMouseJiggler));

    hid_mouse_jiggler->view = view_alloc();
    view_set_context(hid_mouse_jiggler->view, hid_mouse_jiggler);
    view_allocate_model(
        hid_mouse_jiggler->view, ViewModelTypeLocking, sizeof(HidMouseJigglerModel));
    view_set_draw_callback(hid_mouse_jiggler->view, hid_mouse_jiggler_draw_callback);
    view_set_input_callback(hid_mouse_jiggler->view, hid_mouse_jiggler_input_callback);
    view_set_exit_callback(hid_mouse_jiggler->view, hid_mouse_jiggler_exit_callback);

    hid_mouse_jiggler->hid = hid;

    hid_mouse_jiggler->timer = furi_timer_alloc(
        hid_mouse_jiggler_timer_callback, FuriTimerTypePeriodic, hid_mouse_jiggler);

    with_view_model(
        hid_mouse_jiggler->view, HidMouseJigglerModel * model, { model->interval_idx = 2; }, true);

    return hid_mouse_jiggler;
}

void hid_mouse_jiggler_free(HidMouseJiggler* hid_mouse_jiggler) {
    furi_assert(hid_mouse_jiggler);

    furi_timer_stop(hid_mouse_jiggler->timer);
    furi_timer_free(hid_mouse_jiggler->timer);

    view_free(hid_mouse_jiggler->view);

    free(hid_mouse_jiggler);
}

View* hid_mouse_jiggler_get_view(HidMouseJiggler* hid_mouse_jiggler) {
    furi_assert(hid_mouse_jiggler);
    return hid_mouse_jiggler->view;
}

void hid_mouse_jiggler_set_connected_status(HidMouseJiggler* hid_mouse_jiggler, bool connected) {
    furi_assert(hid_mouse_jiggler);
    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerModel * model,
        { model->connected = connected; },
        true);
}
