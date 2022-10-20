#include "gps_uart.h"

#include <furi.h>
#include <gui/gui.h>
#include <string.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

static void render_callback(Canvas* const canvas, void* context) {
    const GpsUart* gps_uart = acquire_mutex((ValueMutex*)context, 25);
    if(gps_uart == NULL) {
        return;
    }

    canvas_set_font(canvas, FontSecondary);
    char buffer[64];
    snprintf(buffer, 64, "LAT: %f", (double)gps_uart->status.latitude);
    canvas_draw_str_aligned(canvas, 10, 10, AlignLeft, AlignBottom, buffer);
    snprintf(buffer, 64, "LON: %f", (double)gps_uart->status.longitude);
    canvas_draw_str_aligned(canvas, 10, 20, AlignLeft, AlignBottom, buffer);
    snprintf(
        buffer,
        64,
        "C/S: %.1f / %.2fkn",
        (double)gps_uart->status.course,
        (double)gps_uart->status.speed);
    canvas_draw_str_aligned(canvas, 10, 30, AlignLeft, AlignBottom, buffer);
    snprintf(
        buffer,
        64,
        "ALT: %.1f %c",
        (double)gps_uart->status.altitude,
        gps_uart->status.altitude_units);
    canvas_draw_str_aligned(canvas, 10, 40, AlignLeft, AlignBottom, buffer);
    snprintf(buffer, 64, "FIX: %d", gps_uart->status.fix_quality);
    canvas_draw_str_aligned(canvas, 10, 50, AlignLeft, AlignBottom, buffer);
    snprintf(buffer, 64, "SAT: %d", gps_uart->status.satellites_tracked);
    canvas_draw_str_aligned(canvas, 10, 60, AlignLeft, AlignBottom, buffer);

    release_mutex((ValueMutex*)context, gps_uart);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

int32_t gps_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    GpsUart* gps_uart = gps_uart_enable();

    ValueMutex gps_uart_mutex;
    if(!init_mutex(&gps_uart_mutex, gps_uart, sizeof(GpsUart))) {
        FURI_LOG_E("GPS", "cannot create mutex\r\n");
        free(gps_uart);
        return 255;
    }

    // set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &gps_uart_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        GpsUart* gps_uart = (GpsUart*)acquire_mutex_block(&gps_uart_mutex);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                    case InputKeyDown:
                    case InputKeyRight:
                    case InputKeyLeft:
                    case InputKeyOk:
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    }
                }
            }
        } else {
            FURI_LOG_D("GPS", "FuriMessageQueue: event timeout");
        }

        view_port_update(view_port);
        release_mutex(&gps_uart_mutex, gps_uart);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&gps_uart_mutex);
    gps_uart_disable(gps_uart);

    return 0;
}
