#include "hid_mouse_jiggler_stealth.h"
#include <gui/elements.h>
#include "../hid.h"

#include "hid_icons.h"

#define TAG "HidMouseJigglerStealth"

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

    // Title "Mouse Jiggler"
    canvas_set_font(canvas, FontPrimary);
#ifdef HID_TRANSPORT_BLE
    elements_multiline_text_aligned(canvas, 17, 4, AlignLeft, AlignTop, "Mouse Jiggler Stealth");
#else
    elements_multiline_text_aligned(canvas, 10, 2, AlignLeft, AlignTop, "Mouse Jiggler Stealth");
#endif

    // Display the current min interval in minutes
    canvas_set_font(canvas, FontSecondary); // Assuming there's a smaller font available
    FuriString* min_interval_str = furi_string_alloc_printf("Min: %d min", model->min_interval);
    elements_multiline_text_aligned(
        canvas, 0, 16, AlignLeft, AlignTop, furi_string_get_cstr(min_interval_str));
    furi_string_free(min_interval_str);

    // Display the current max interval in minutes
    FuriString* max_interval_str = furi_string_alloc_printf("Max: %d min", model->max_interval);
    elements_multiline_text_aligned(
        canvas, 0, 28, AlignLeft, AlignTop, furi_string_get_cstr(max_interval_str));
    furi_string_free(max_interval_str);

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

                // Randomize the mouse movement distance and direction
                int move_x = (rand() % 2001) - 1000; // Randomly between -1000 and 1000
                int move_y = (rand() % 2001) - 1000; // Randomly between -1000 and 1000

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
                    if(!model->running && model->min_interval > 1) { // Minimum 1 minute
                        model->min_interval--; // Decrement min interval by 1 minute
                    }
                    consumed = true;
                    break;

                case InputKeyRight:
                    if(!model->running && model->max_interval < 30) { // Maximum 30 minutes
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
            // Initialize the min and max interval values
            model->min_interval = 1; // 1 minutes
            model->max_interval = 4; // 4 minutes
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
