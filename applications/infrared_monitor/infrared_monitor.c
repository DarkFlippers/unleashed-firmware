#include <gui/canvas.h>
#include <input/input.h>
#include <infrared.h>
#include <infrared_worker.h>
#include <stdio.h>
#include <furi.h>
#include <furi_hal_infrared.h>
#include <furi_hal.h>
#include <gui/view_port.h>
#include <gui/gui.h>
#include <gui/elements.h>

#define INFRARED_TIMINGS_SIZE 700

typedef struct {
    uint32_t timing_cnt;
    struct {
        uint8_t level;
        uint32_t duration;
    } timing[INFRARED_TIMINGS_SIZE];
} InfraredDelaysArray;

typedef struct {
    char display_text[64];
    osMessageQueueId_t event_queue;
    InfraredDelaysArray delays;
    InfraredWorker* worker;
    ViewPort* view_port;
} InfraredMonitor;

void infrared_monitor_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    InfraredMonitor* infrared_monitor = (InfraredMonitor*)ctx;

    if((input_event->type == InputTypeShort) && (input_event->key == InputKeyBack)) {
        osMessageQueuePut(infrared_monitor->event_queue, input_event, 0, 0);
    }
}

static void infrared_monitor_draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(canvas);
    furi_assert(ctx);
    InfraredMonitor* infrared_monitor = (InfraredMonitor*)ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 64, 0, AlignCenter, AlignTop, "INFRARED monitor\n");
    canvas_set_font(canvas, FontKeyboard);
    if(strlen(infrared_monitor->display_text)) {
        elements_multiline_text_aligned(
            canvas, 64, 43, AlignCenter, AlignCenter, infrared_monitor->display_text);
    }
}

static void signal_received_callback(void* context, InfraredWorkerSignal* received_signal) {
    furi_assert(context);
    furi_assert(received_signal);
    InfraredMonitor* infrared_monitor = context;

    if(infrared_worker_signal_is_decoded(received_signal)) {
        const InfraredMessage* message = infrared_worker_get_decoded_signal(received_signal);
        snprintf(
            infrared_monitor->display_text,
            sizeof(infrared_monitor->display_text),
            "%s\nA:0x%0*lX\nC:0x%0*lX\n%s\n",
            infrared_get_protocol_name(message->protocol),
            ROUND_UP_TO(infrared_get_protocol_address_length(message->protocol), 4),
            message->address,
            ROUND_UP_TO(infrared_get_protocol_command_length(message->protocol), 4),
            message->command,
            message->repeat ? " R" : "");
        view_port_update(infrared_monitor->view_port);
        printf(
            "== %s, A:0x%0*lX, C:0x%0*lX%s ==\r\n",
            infrared_get_protocol_name(message->protocol),
            ROUND_UP_TO(infrared_get_protocol_address_length(message->protocol), 4),
            message->address,
            ROUND_UP_TO(infrared_get_protocol_command_length(message->protocol), 4),
            message->command,
            message->repeat ? " R" : "");
    } else {
        const uint32_t* timings;
        size_t timings_cnt;
        infrared_worker_get_raw_signal(received_signal, &timings, &timings_cnt);
        snprintf(
            infrared_monitor->display_text,
            sizeof(infrared_monitor->display_text),
            "RAW\n%d samples\n",
            timings_cnt);
        view_port_update(infrared_monitor->view_port);
        printf("RAW, %d samples:\r\n", timings_cnt);
        for(size_t i = 0; i < timings_cnt; ++i) {
            printf("%lu ", timings[i]);
        }
        printf("\r\n");
    }
}

int32_t infrared_monitor_app(void* p) {
    (void)p;

    InfraredMonitor* infrared_monitor = malloc(sizeof(InfraredMonitor));
    infrared_monitor->display_text[0] = 0;
    infrared_monitor->event_queue = osMessageQueueNew(1, sizeof(InputEvent), NULL);
    infrared_monitor->view_port = view_port_alloc();
    Gui* gui = furi_record_open("gui");

    view_port_draw_callback_set(
        infrared_monitor->view_port, infrared_monitor_draw_callback, infrared_monitor);
    view_port_input_callback_set(
        infrared_monitor->view_port, infrared_monitor_input_callback, infrared_monitor);

    gui_add_view_port(gui, infrared_monitor->view_port, GuiLayerFullscreen);

    infrared_monitor->worker = infrared_worker_alloc();
    infrared_worker_rx_start(infrared_monitor->worker);
    infrared_worker_rx_set_received_signal_callback(
        infrared_monitor->worker, signal_received_callback, infrared_monitor);
    infrared_worker_rx_enable_blink_on_receiving(infrared_monitor->worker, true);

    while(1) {
        InputEvent event;
        if(osOK == osMessageQueueGet(infrared_monitor->event_queue, &event, NULL, 50)) {
            if((event.type == InputTypeShort) && (event.key == InputKeyBack)) {
                break;
            }
        }
    }

    infrared_worker_rx_stop(infrared_monitor->worker);
    infrared_worker_free(infrared_monitor->worker);
    osMessageQueueDelete(infrared_monitor->event_queue);
    view_port_enabled_set(infrared_monitor->view_port, false);
    gui_remove_view_port(gui, infrared_monitor->view_port);
    view_port_free(infrared_monitor->view_port);
    furi_record_close("gui");
    free(infrared_monitor);

    return 0;
}
