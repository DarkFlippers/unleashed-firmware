#include <gui/canvas.h>
#include <input/input.h>
#include <irda.h>
#include <irda_worker.h>
#include <stdio.h>
#include <furi.h>
#include <api-hal-irda.h>
#include <api-hal.h>
#include <gui/view_port.h>
#include <gui/gui.h>
#include <gui/elements.h>

#define IRDA_TIMINGS_SIZE 700

typedef struct {
    uint32_t timing_cnt;
    struct {
        uint8_t level;
        uint32_t duration;
    } timing[IRDA_TIMINGS_SIZE];
} IrdaDelaysArray;

typedef struct {
    char display_text[64];
    osMessageQueueId_t event_queue;
    IrdaDelaysArray delays;
    IrdaWorker* worker;
    ViewPort* view_port;
} IrdaMonitor;

void irda_monitor_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    IrdaMonitor* irda_monitor = (IrdaMonitor*)ctx;

    if((input_event->type == InputTypeShort) && (input_event->key == InputKeyBack)) {
        osMessageQueuePut(irda_monitor->event_queue, input_event, 0, 0);
    }
}

static void irda_monitor_draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(canvas);
    furi_assert(ctx);
    IrdaMonitor* irda_monitor = (IrdaMonitor*)ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 64, 0, AlignCenter, AlignTop, "IRDA monitor\n");
    canvas_set_font(canvas, FontKeyboard);
    if(strlen(irda_monitor->display_text)) {
        elements_multiline_text_aligned(
            canvas, 64, 43, AlignCenter, AlignCenter, irda_monitor->display_text);
    }
}

static void signal_received_callback(void* context, IrdaWorkerSignal* received_signal) {
    furi_assert(context);
    furi_assert(received_signal);
    IrdaMonitor* irda_monitor = context;

    if(irda_worker_signal_is_decoded(received_signal)) {
        const IrdaMessage* message = irda_worker_get_decoded_message(received_signal);
        snprintf(
            irda_monitor->display_text,
            sizeof(irda_monitor->display_text),
            "%s\nA:0x%0*lX\nC:0x%0*lX\n%s\n",
            irda_get_protocol_name(message->protocol),
            irda_get_protocol_address_length(message->protocol),
            message->address,
            irda_get_protocol_command_length(message->protocol),
            message->command,
            message->repeat ? " R" : "");
        view_port_update(irda_monitor->view_port);
        printf(
            "== %s, A:0x%0*lX, C:0x%0*lX%s ==\r\n",
            irda_get_protocol_name(message->protocol),
            irda_get_protocol_address_length(message->protocol),
            message->address,
            irda_get_protocol_command_length(message->protocol),
            message->command,
            message->repeat ? " R" : "");
    } else {
        const uint32_t* timings;
        size_t timings_cnt;
        irda_worker_get_raw_signal(received_signal, &timings, &timings_cnt);
        snprintf(
            irda_monitor->display_text,
            sizeof(irda_monitor->display_text),
            "RAW\n%d samples\n",
            timings_cnt);
        view_port_update(irda_monitor->view_port);
        printf("RAW, %d samples:\r\n", timings_cnt);
        for(size_t i = 0; i < timings_cnt; ++i) {
            printf("%lu ", timings[i]);
        }
        printf("\r\n");
    }
}

int32_t irda_monitor_app(void* p) {
    (void)p;

    IrdaMonitor* irda_monitor = furi_alloc(sizeof(IrdaMonitor));
    irda_monitor->display_text[0] = 0;
    irda_monitor->event_queue = osMessageQueueNew(1, sizeof(InputEvent), NULL);
    irda_monitor->view_port = view_port_alloc();
    Gui* gui = furi_record_open("gui");

    view_port_draw_callback_set(irda_monitor->view_port, irda_monitor_draw_callback, irda_monitor);
    view_port_input_callback_set(
        irda_monitor->view_port, irda_monitor_input_callback, irda_monitor);

    gui_add_view_port(gui, irda_monitor->view_port, GuiLayerFullscreen);

    irda_monitor->worker = irda_worker_alloc();
    irda_worker_set_context(irda_monitor->worker, irda_monitor);
    irda_worker_start(irda_monitor->worker);
    irda_worker_set_received_signal_callback(irda_monitor->worker, signal_received_callback);
    irda_worker_enable_blink_on_receiving(irda_monitor->worker, true);

    while(1) {
        InputEvent event;
        if(osOK == osMessageQueueGet(irda_monitor->event_queue, &event, NULL, 50)) {
            if((event.type == InputTypeShort) && (event.key == InputKeyBack)) {
                break;
            }
        }
    }

    irda_worker_stop(irda_monitor->worker);
    irda_worker_free(irda_monitor->worker);
    osMessageQueueDelete(irda_monitor->event_queue);
    view_port_enabled_set(irda_monitor->view_port, false);
    gui_remove_view_port(gui, irda_monitor->view_port);
    view_port_free(irda_monitor->view_port);
    furi_record_close("gui");
    free(irda_monitor);

    return 0;
}
