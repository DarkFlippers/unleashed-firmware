#include "hid_mouse_clicker.h"
#include <gui/elements.h>
#include "../hid.h"

#include "hid_icons.h"

#define TAG "HidMouseClicker"
#define DEFAULT_CLICK_RATE 1
#define MAXIMUM_CLICK_RATE 60

struct HidMouseClicker {
    View* view;
    Hid* hid;
    FuriTimer* timer;
};

typedef struct {
    bool connected;
    bool running;
    int rate;
    HidTransport transport;
} HidMouseClickerModel;

static void hid_mouse_clicker_start_or_restart_timer(void* context) {
    furi_assert(context);
    HidMouseClicker* hid_mouse_clicker = context;

    if(furi_timer_is_running(hid_mouse_clicker->timer)) {
        furi_timer_stop(hid_mouse_clicker->timer);
    }

    with_view_model(
        hid_mouse_clicker->view,
        HidMouseClickerModel * model,
        {
            furi_timer_start(
                hid_mouse_clicker->timer, furi_kernel_get_tick_frequency() / model->rate);
        },
        true);
}

static void hid_mouse_clicker_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidMouseClickerModel* model = context;

    // Header
    if(model->transport == HidTransportBle) {
        if(model->connected) {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
        } else {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
        }
    }

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Mouse Clicker");

    // Ok
    canvas_draw_icon(canvas, 63, 25, &I_Space_65x18);
    if(model->running) {
        canvas_set_font(canvas, FontPrimary);

        FuriString* rate_label = furi_string_alloc();
        furi_string_printf(rate_label, "%d clicks/s\n\nUp / Down", model->rate);
        elements_multiline_text(canvas, AlignLeft, 35, furi_string_get_cstr(rate_label));
        canvas_set_font(canvas, FontSecondary);
        furi_string_free(rate_label);

        elements_slightly_rounded_box(canvas, 66, 27, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text(canvas, AlignLeft, 35, "Press Start\nto start\nclicking");
        canvas_set_font(canvas, FontSecondary);
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

static void hid_mouse_clicker_timer_callback(void* context) {
    furi_assert(context);
    HidMouseClicker* hid_mouse_clicker = context;
    with_view_model(
        hid_mouse_clicker->view,
        HidMouseClickerModel * model,
        {
            if(model->running) {
                hid_hal_mouse_press(hid_mouse_clicker->hid, HID_MOUSE_BTN_LEFT);
                hid_hal_mouse_release(hid_mouse_clicker->hid, HID_MOUSE_BTN_LEFT);
            }
        },
        false);
}

static void hid_mouse_clicker_enter_callback(void* context) {
    hid_mouse_clicker_start_or_restart_timer(context);
}

static void hid_mouse_clicker_exit_callback(void* context) {
    furi_assert(context);
    HidMouseClicker* hid_mouse_clicker = context;
    furi_timer_stop(hid_mouse_clicker->timer);
}

static bool hid_mouse_clicker_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidMouseClicker* hid_mouse_clicker = context;

    bool consumed = false;
    bool rate_changed = false;

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) {
        return false;
    }

    with_view_model(
        hid_mouse_clicker->view,
        HidMouseClickerModel * model,
        {
            switch(event->key) {
            case InputKeyOk:
                model->running = !model->running;
                consumed = true;
                break;
            case InputKeyUp:
                if(model->rate < MAXIMUM_CLICK_RATE) {
                    model->rate++;
                }
                rate_changed = true;
                consumed = true;
                break;
            case InputKeyDown:
                if(model->rate > 1) {
                    model->rate--;
                }
                rate_changed = true;
                consumed = true;
                break;
            default:
                consumed = true;
                break;
            }
        },
        true);

    if(rate_changed) {
        hid_mouse_clicker_start_or_restart_timer(context);
    }

    return consumed;
}

HidMouseClicker* hid_mouse_clicker_alloc(Hid* hid) {
    HidMouseClicker* hid_mouse_clicker = malloc(sizeof(HidMouseClicker));

    hid_mouse_clicker->view = view_alloc();
    view_set_context(hid_mouse_clicker->view, hid_mouse_clicker);
    view_allocate_model(
        hid_mouse_clicker->view, ViewModelTypeLocking, sizeof(HidMouseClickerModel));
    view_set_draw_callback(hid_mouse_clicker->view, hid_mouse_clicker_draw_callback);
    view_set_input_callback(hid_mouse_clicker->view, hid_mouse_clicker_input_callback);
    view_set_enter_callback(hid_mouse_clicker->view, hid_mouse_clicker_enter_callback);
    view_set_exit_callback(hid_mouse_clicker->view, hid_mouse_clicker_exit_callback);

    hid_mouse_clicker->hid = hid;

    hid_mouse_clicker->timer = furi_timer_alloc(
        hid_mouse_clicker_timer_callback, FuriTimerTypePeriodic, hid_mouse_clicker);

    with_view_model(
        hid_mouse_clicker->view,
        HidMouseClickerModel * model,
        {
            model->transport = hid->transport;
            model->rate = DEFAULT_CLICK_RATE;
        },
        true);

    return hid_mouse_clicker;
}

void hid_mouse_clicker_free(HidMouseClicker* hid_mouse_clicker) {
    furi_assert(hid_mouse_clicker);

    furi_timer_stop(hid_mouse_clicker->timer);
    furi_timer_free(hid_mouse_clicker->timer);

    view_free(hid_mouse_clicker->view);

    free(hid_mouse_clicker);
}

View* hid_mouse_clicker_get_view(HidMouseClicker* hid_mouse_clicker) {
    furi_assert(hid_mouse_clicker);
    return hid_mouse_clicker->view;
}

void hid_mouse_clicker_set_connected_status(HidMouseClicker* hid_mouse_clicker, bool connected) {
    furi_assert(hid_mouse_clicker);
    with_view_model(
        hid_mouse_clicker->view,
        HidMouseClickerModel * model,
        { model->connected = connected; },
        true);
}
