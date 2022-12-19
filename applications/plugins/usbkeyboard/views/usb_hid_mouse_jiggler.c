#include "usb_hid_mouse_jiggler.h"
#include <furi.h>
#include <furi_hal_usb_hid.h>
#include <gui/elements.h>

#include <USB_Keyboard_icons.h>

#define TAG "HidMouseJiggler"

struct HidMouseJiggler {
    View* view;
    FuriTimer* timer;
};

typedef struct {
    bool running;
    uint8_t counter;
} HidMouseJigglerModel;

static void hid_mouse_jiggler_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidMouseJigglerModel* model = context;

    // Header
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Mouse Jiggler");

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text(canvas, AlignLeft, 35, "Press Start\nto jiggle");
    canvas_set_font(canvas, FontSecondary);

    // Ok
    canvas_draw_icon(canvas, 63, 25, &I_Space_65x18);
    if(model->running) {
        elements_slightly_rounded_box(canvas, 66, 27, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 74, 29, &I_Ok_btn_9x9);
    if(model->running) {
        elements_multiline_text_aligned(canvas, 91, 36, AlignLeft, AlignBottom, "Stop");
    } else {
        elements_multiline_text_aligned(canvas, 91, 36, AlignLeft, AlignBottom, "Start");
    }
    canvas_set_color(canvas, ColorBlack);

    // Back
    canvas_draw_icon(canvas, 74, 49, &I_Pin_back_arrow_10x8);
    elements_multiline_text_aligned(canvas, 91, 57, AlignLeft, AlignBottom, "Quit");
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
                furi_hal_hid_mouse_move(
                    (model->counter % 2 == 0) ? MOUSE_MOVE_SHORT : -MOUSE_MOVE_SHORT, 0);
            }
        },
        false);
}

static void hid_mouse_jiggler_enter_callback(void* context) {
    furi_assert(context);
    HidMouseJiggler* hid_mouse_jiggler = context;

    furi_timer_start(hid_mouse_jiggler->timer, 500);
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

    if(event->key == InputKeyOk) {
        with_view_model(
            hid_mouse_jiggler->view,
            HidMouseJigglerModel * model,
            { model->running = !model->running; },
            true);
        consumed = true;
    }

    return consumed;
}

HidMouseJiggler* hid_mouse_jiggler_alloc() {
    HidMouseJiggler* hid_mouse_jiggler = malloc(sizeof(HidMouseJiggler));

    hid_mouse_jiggler->view = view_alloc();
    view_set_context(hid_mouse_jiggler->view, hid_mouse_jiggler);
    view_allocate_model(
        hid_mouse_jiggler->view, ViewModelTypeLocking, sizeof(HidMouseJigglerModel));
    view_set_draw_callback(hid_mouse_jiggler->view, hid_mouse_jiggler_draw_callback);
    view_set_input_callback(hid_mouse_jiggler->view, hid_mouse_jiggler_input_callback);
    view_set_enter_callback(hid_mouse_jiggler->view, hid_mouse_jiggler_enter_callback);
    view_set_exit_callback(hid_mouse_jiggler->view, hid_mouse_jiggler_exit_callback);

    hid_mouse_jiggler->timer = furi_timer_alloc(
        hid_mouse_jiggler_timer_callback, FuriTimerTypePeriodic, hid_mouse_jiggler);

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
