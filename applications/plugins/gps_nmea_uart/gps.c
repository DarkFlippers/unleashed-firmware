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

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 32, 8, AlignCenter, AlignBottom, "Latitude");
    canvas_draw_str_aligned(canvas, 96, 8, AlignCenter, AlignBottom, "Longitude");
    canvas_draw_str_aligned(canvas, 21, 30, AlignCenter, AlignBottom, "Course");
    canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignBottom, "Speed");
    canvas_draw_str_aligned(canvas, 107, 30, AlignCenter, AlignBottom, "Altitude");
    canvas_draw_str_aligned(canvas, 32, 52, AlignCenter, AlignBottom, "Satellites");
    canvas_draw_str_aligned(canvas, 96, 52, AlignCenter, AlignBottom, "Last Fix");

    canvas_set_font(canvas, FontSecondary);
    char buffer[64];
    snprintf(buffer, 64, "%f", (double)gps_uart->status.latitude);
    canvas_draw_str_aligned(canvas, 32, 18, AlignCenter, AlignBottom, buffer);
    snprintf(buffer, 64, "%f", (double)gps_uart->status.longitude);
    canvas_draw_str_aligned(canvas, 96, 18, AlignCenter, AlignBottom, buffer);
    snprintf(buffer, 64, "%.1f", (double)gps_uart->status.course);
    canvas_draw_str_aligned(canvas, 21, 40, AlignCenter, AlignBottom, buffer);
    snprintf(buffer, 64, "%.2f kn", (double)gps_uart->status.speed);
    canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignBottom, buffer);
    snprintf(
        buffer,
        64,
        "%.1f %c",
        (double)gps_uart->status.altitude,
        tolower(gps_uart->status.altitude_units));
    canvas_draw_str_aligned(canvas, 107, 40, AlignCenter, AlignBottom, buffer);
    snprintf(buffer, 64, "%d", gps_uart->status.satellites_tracked);
    canvas_draw_str_aligned(canvas, 32, 62, AlignCenter, AlignBottom, buffer);
    snprintf(
        buffer,
        64,
        "%02d:%02d:%02d UTC",
        gps_uart->status.time_hours,
        gps_uart->status.time_minutes,
        gps_uart->status.time_seconds);
    canvas_draw_str_aligned(canvas, 96, 62, AlignCenter, AlignBottom, buffer);

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
