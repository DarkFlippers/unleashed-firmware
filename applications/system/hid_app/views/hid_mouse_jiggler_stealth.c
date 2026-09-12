#include "hid_mouse_jiggler_stealth.h"
#include <stdint.h>
#include <gui/elements.h>
#include "../hid.h"

#include "hid_icons.h"

#define TAG "HidMouseJigglerStealth"

#define INTERVAL_MIN_MINUTES 1
#define INTERVAL_MAX_MINUTES 30

struct HidMouseJigglerStealth {
    View* view;
    Hid* hid;
    FuriTimer* timer;
};

typedef struct {
    bool connected;
    bool running;
    int min_interval; // Minimum interval for random range
    int max_interval; // Maximum interval for random range
} HidMouseJigglerStealthModel;

static void hid_mouse_jiggler_stealth_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidMouseJigglerStealthModel* model = context;

// Header
#ifdef HID_TRANSPORT_BLE
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }
#endif

    canvas_set_font(canvas, FontPrimary);
#ifdef HID_TRANSPORT_BLE
    elements_multiline_text_aligned(canvas, 17, 4, AlignLeft, AlignTop, "Mouse Jiggler Stealth");
#else
    elements_multiline_text_aligned(canvas, 10, 2, AlignLeft, AlignTop, "Mouse Jiggler Stealth");
#endif

    // Both rows hint only presses that do something - keep bounds in sync with the input handler
    canvas_set_font(canvas, FontSecondary);
    FuriString* min_interval_str = furi_string_alloc_printf("Min:%dm", model->min_interval);
    elements_multiline_text_aligned(
        canvas, 0, 16, AlignLeft, AlignTop, furi_string_get_cstr(min_interval_str));
    furi_string_free(min_interval_str);
    if(!model->running) {
        if(model->min_interval < model->max_interval)
            canvas_draw_icon(canvas, 48, 18, &I_ButtonUp_7x4);
        if(model->min_interval > INTERVAL_MIN_MINUTES)
            canvas_draw_icon(canvas, 57, 18, &I_ButtonDown_7x4);
    }

    FuriString* max_interval_str = furi_string_alloc_printf("Max:%dm", model->max_interval);
    elements_multiline_text_aligned(
        canvas, 0, 28, AlignLeft, AlignTop, furi_string_get_cstr(max_interval_str));
    furi_string_free(max_interval_str);
    if(!model->running) {
        if(model->max_interval > model->min_interval + 1)
            canvas_draw_icon(canvas, 48, 28, &I_ButtonLeft_4x7);
        if(model->max_interval < INTERVAL_MAX_MINUTES)
            canvas_draw_icon(canvas, 57, 28, &I_ButtonRight_4x7);
    }

    // "Press Start to jiggle"
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text(canvas, AlignLeft, 50, "Press Start\nto jiggle");

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

static void hid_mouse_jiggler_stealth_timer_callback(void* context) {
    furi_assert(context);
    HidMouseJigglerStealth* hid_mouse_jiggler = context;
    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerStealthModel * model,
        {
            if(model->running) {
                // Generate a random interval in minutes and convert to milliseconds
                int randomIntervalMinutes =
                    model->min_interval + rand() % (model->max_interval - model->min_interval + 1);

                // HID mouse reports use signed 8-bit movement deltas
                int8_t move_x = (rand() % (2 * INT8_MAX + 1)) - INT8_MAX;
                int8_t move_y = (rand() % (2 * INT8_MAX + 1)) - INT8_MAX;

                // Perform the mouse move with the randomized values
                hid_hal_mouse_move(hid_mouse_jiggler->hid, move_x, move_y);

                // Restart timer with the new random interval
                furi_timer_stop(hid_mouse_jiggler->timer);
                furi_timer_start(hid_mouse_jiggler->timer, randomIntervalMinutes * 60000);
            }
        },
        false);
}

static void hid_mouse_jiggler_stealth_exit_callback(void* context) {
    furi_assert(context);
    HidMouseJigglerStealth* hid_mouse_jiggler = context;
    furi_timer_stop(hid_mouse_jiggler->timer);
}

static bool hid_mouse_jiggler_stealth_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidMouseJigglerStealth* hid_mouse_jiggler = context;

    bool consumed = false;

    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerStealthModel * model,
        {
            if(event->type == InputTypePress) {
                switch(event->key) {
                case InputKeyOk:
                    model->running = !model->running;
                    if(model->running) {
                        furi_timer_stop(hid_mouse_jiggler->timer);
                        int randomIntervalMinutes =
                            model->min_interval +
                            rand() % (model->max_interval - model->min_interval + 1);
                        furi_timer_start(hid_mouse_jiggler->timer, randomIntervalMinutes * 60000);
                    }
                    consumed = true;
                    break;

                case InputKeyUp:
                    if(!model->running && model->min_interval < model->max_interval) {
                        model->min_interval++; // Increment min interval by 1 minute
                    }
                    consumed = true;
                    break;

                case InputKeyDown:
                    if(!model->running && model->min_interval > INTERVAL_MIN_MINUTES) {
                        model->min_interval--; // Decrement min interval by 1 minute
                    }
                    consumed = true;
                    break;

                case InputKeyRight:
                    if(!model->running && model->max_interval < INTERVAL_MAX_MINUTES) {
                        model->max_interval++; // Increment max interval by 1 minute
                    }
                    consumed = true;
                    break;

                case InputKeyLeft:
                    if(!model->running && model->max_interval > model->min_interval + 1) {
                        model->max_interval--; // Decrement max interval by 1 minute
                    }
                    consumed = true;
                    break;

                default:
                    break;
                }
            }
        },
        true);

    return consumed;
}

HidMouseJigglerStealth* hid_mouse_jiggler_stealth_alloc(Hid* hid) {
    HidMouseJigglerStealth* hid_mouse_jiggler = malloc(sizeof(HidMouseJigglerStealth));

    hid_mouse_jiggler->view = view_alloc();
    view_set_context(hid_mouse_jiggler->view, hid_mouse_jiggler);
    view_allocate_model(
        hid_mouse_jiggler->view, ViewModelTypeLocking, sizeof(HidMouseJigglerStealthModel));
    view_set_draw_callback(hid_mouse_jiggler->view, hid_mouse_jiggler_stealth_draw_callback);
    view_set_input_callback(hid_mouse_jiggler->view, hid_mouse_jiggler_stealth_input_callback);
    view_set_exit_callback(hid_mouse_jiggler->view, hid_mouse_jiggler_stealth_exit_callback);

    hid_mouse_jiggler->hid = hid;

    hid_mouse_jiggler->timer = furi_timer_alloc(
        hid_mouse_jiggler_stealth_timer_callback, FuriTimerTypePeriodic, hid_mouse_jiggler);

    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerStealthModel * model,
        {
            // Default random range, in minutes
            model->min_interval = 1;
            model->max_interval = 4;
        },
        true);

    return hid_mouse_jiggler;
}

void hid_mouse_jiggler_stealth_free(HidMouseJigglerStealth* hid_mouse_jiggler) {
    furi_assert(hid_mouse_jiggler);

    furi_timer_stop(hid_mouse_jiggler->timer);
    furi_timer_free(hid_mouse_jiggler->timer);

    view_free(hid_mouse_jiggler->view);

    free(hid_mouse_jiggler);
}

View* hid_mouse_jiggler_stealth_get_view(HidMouseJigglerStealth* hid_mouse_jiggler) {
    furi_assert(hid_mouse_jiggler);
    return hid_mouse_jiggler->view;
}

void hid_mouse_jiggler_stealth_set_connected_status(
    HidMouseJigglerStealth* hid_mouse_jiggler,
    bool connected) {
    furi_assert(hid_mouse_jiggler);
    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerStealthModel * model,
        { model->connected = connected; },
        true);
}
