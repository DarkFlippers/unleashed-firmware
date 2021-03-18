#include <furi.h>
#include <input/input.h>
#include <gui/gui.h>
#include <api-hal.h>

#include "u8g2/u8g2.h"

extern TIM_HandleTypeDef SPEAKER_TIM;

bool exit_app;

static void event_cb(const void* value, void* ctx) {
    furi_assert(value);
    const InputEvent* event = value;
    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        exit_app = true;
    }
}

void coreglitch_draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Coreglitch demo application");
}

int32_t coreglitch_demo_0(void* p) {
    printf("coreglitch demo!\r\n");

    exit_app = false;
    PubSub* event_record = furi_record_open("input_events");
    PubSubItem* event_pubsub = subscribe_pubsub(event_record, event_cb, NULL);

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    furi_check(view_port);
    view_port_draw_callback_set(view_port, coreglitch_draw_callback, NULL);

    // Register view port in GUI
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    float notes[] = {
        0.0,
        330.0,
        220.0,
        0.0,
        110.0 + 55.0,
        440.0,
        330.0,
        55.0,
    };

    float scales[] = {
        1.0,
        1.5,
        0.75,
        0.8,
    };

    uint8_t cnt = 0;

    while(1) {
        for(size_t note_idx = 0; (note_idx < 400) && (!exit_app); note_idx++) {
            float scale = scales[((cnt + note_idx) / 16) % 4];

            float freq = notes[(note_idx + cnt / 2) % 8] * scale;
            float width = 0.001 + 0.05 * (note_idx % (cnt / 7 + 5));

            if(note_idx % 8 == 0) {
                freq = 0;
            }

            // TODO get sound from FURI
            hal_pwm_set(width, freq, &SPEAKER_TIM, SPEAKER_CH);

            // delay(1);

            cnt++;
            delay(100);
        }
        if(exit_app) {
            break;
        }
    }
    hal_pwm_stop(&SPEAKER_TIM, SPEAKER_CH);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    unsubscribe_pubsub(event_pubsub);

    return 0;
}
