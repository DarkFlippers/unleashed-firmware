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

static uint32_t hid_mouse_jiggler_stealth_interval_to_ticks(int interval_minutes) {
    uint32_t interval_ms = (uint32_t)interval_minutes * 60000U;
    uint32_t ticks = furi_ms_to_ticks(interval_ms);
    return ticks ? ticks : 1;
}

static int8_t hid_mouse_jiggler_stealth_random_move(void) {
    // Mouse HID reports carry signed 8-bit relative movement deltas.
    return (int8_t)((rand() % (2 * INT8_MAX + 1)) - INT8_MAX);
}

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
#ifdef HID_TRANSPORT_BLE
    if(model->running && !model->connected) {
        elements_multiline_text(canvas, AlignLeft, 50, "Waiting for\nBluetooth");
    } else {
        elements_multiline_text(canvas, AlignLeft, 50, "Press Start\nto jiggle");
    }
#else
    elements_multiline_text(canvas, AlignLeft, 50, "Press Start\nto jiggle");
#endif

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
    bool running = false;
    bool connected = true;
    int8_t move_x = 0;
    int8_t move_y = 0;
    uint32_t timer_period = 0;

    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerStealthModel * model,
        {
            if(model->running) {
                running = true;
#ifdef HID_TRANSPORT_BLE
                connected = model->connected;
#endif

                // Keep the next random interval in kernel ticks for the Furi timer.
                int randomIntervalMinutes =
                    model->min_interval + rand() % (model->max_interval - model->min_interval + 1);
                timer_period = hid_mouse_jiggler_stealth_interval_to_ticks(randomIntervalMinutes);

                if(connected) {
                    do {
                        move_x = hid_mouse_jiggler_stealth_random_move();
                        move_y = hid_mouse_jiggler_stealth_random_move();
                    } while(move_x == 0 && move_y == 0);
                }
            }
        },
        false);

    if(!running) return;

    if(connected) {
        hid_hal_mouse_move(hid_mouse_jiggler->hid, move_x, move_y);
    }

    furi_timer_start(hid_mouse_jiggler->timer, timer_period);
}

static void hid_mouse_jiggler_stealth_exit_callback(void* context) {
    furi_assert(context);
    HidMouseJigglerStealth* hid_mouse_jiggler = context;
    furi_timer_stop(hid_mouse_jiggler->timer);
    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerStealthModel * model,
        { model->running = false; },
        false);
}

static bool hid_mouse_jiggler_stealth_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidMouseJigglerStealth* hid_mouse_jiggler = context;

#ifdef HID_TRANSPORT_BLE
    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        view_dispatcher_send_custom_event(
            hid_mouse_jiggler->hid->view_dispatcher, HidCustomEventUnpair);
        return true;
    }
#endif

    bool consumed = false;
    bool timer_start = false;
    uint32_t timer_period = 0;

    if(event->type == InputTypePress && event->key == InputKeyOk) {
        furi_timer_stop(hid_mouse_jiggler->timer);
    }

    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerStealthModel * model,
        {
            if(event->type == InputTypePress) {
                switch(event->key) {
                case InputKeyOk:
                    model->running = !model->running;
                    if(model->running) {
                        int randomIntervalMinutes =
                            model->min_interval +
                            rand() % (model->max_interval - model->min_interval + 1);
                        timer_period =
                            hid_mouse_jiggler_stealth_interval_to_ticks(randomIntervalMinutes);
                        timer_start = true;
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

    if(timer_start) {
        furi_timer_start(hid_mouse_jiggler->timer, timer_period);
    }

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
        hid_mouse_jiggler_stealth_timer_callback, FuriTimerTypeOnce, hid_mouse_jiggler);

    with_view_model(
        hid_mouse_jiggler->view,
        HidMouseJigglerStealthModel * model,
        {
            // Default random range, in minutes
            model->connected = false;
            model->running = false;
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
