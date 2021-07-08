#include "gui/canvas.h"
#include "irda.h"
#include <stdio.h>
#include <furi.h>
#include <api-hal-irda.h>
#include <api-hal.h>
#include <notification/notification-messages.h>
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
    IrdaDecoderHandler* handler;
    char display_text[64];
    osMessageQueueId_t event_queue;
    IrdaDelaysArray delays;
} IrdaMonitor;

static void irda_rx_callback(void* ctx, bool level, uint32_t duration) {
    IrdaMonitor* irda_monitor = (IrdaMonitor*)ctx;
    IrdaDelaysArray* delays = &irda_monitor->delays;

    if(delays->timing_cnt > 1) furi_assert(level != delays->timing[delays->timing_cnt - 1].level);
    delays->timing[delays->timing_cnt].level = level;
    delays->timing[delays->timing_cnt].duration = duration;
    delays->timing_cnt++; // Read-Modify-Write in ISR only: no need to add synchronization
    if(delays->timing_cnt >= IRDA_TIMINGS_SIZE) {
        delays->timing_cnt = 0;
    }
}

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

int32_t irda_monitor_app(void* p) {
    (void)p;
    uint32_t counter = 0;
    uint32_t print_counter = 0;

    IrdaMonitor* irda_monitor = furi_alloc(sizeof(IrdaMonitor));
    irda_monitor->display_text[0] = 0;
    irda_monitor->event_queue = osMessageQueueNew(1, sizeof(InputEvent), NULL);
    ViewPort* view_port = view_port_alloc();
    IrdaDelaysArray* delays = &irda_monitor->delays;
    NotificationApp* notification = furi_record_open("notification");
    Gui* gui = furi_record_open("gui");

    view_port_draw_callback_set(view_port, irda_monitor_draw_callback, irda_monitor);
    view_port_input_callback_set(view_port, irda_monitor_input_callback, irda_monitor);

    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    api_hal_irda_rx_irq_init();
    api_hal_irda_rx_irq_set_callback(irda_rx_callback, irda_monitor);
    irda_monitor->handler = irda_alloc_decoder();

    while(1) {
        InputEvent event;
        if(osOK == osMessageQueueGet(irda_monitor->event_queue, &event, NULL, 50)) {
            if((event.type == InputTypeShort) && (event.key == InputKeyBack)) {
                break;
            }
        }

        if(counter != delays->timing_cnt) {
            notification_message(notification, &sequence_blink_blue_10);
        }

        for(; counter != delays->timing_cnt;) {
            const IrdaMessage* message = irda_decode(
                irda_monitor->handler,
                delays->timing[counter].level,
                delays->timing[counter].duration);

            ++counter;
            if(counter >= IRDA_TIMINGS_SIZE) counter = 0;

            if(message) {
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
                view_port_update(view_port);
            }

            size_t distance = (counter > print_counter) ?
                                  counter - print_counter :
                                  (counter + IRDA_TIMINGS_SIZE) - print_counter;
            if(message || (distance > (IRDA_TIMINGS_SIZE / 2))) {
                if(message) {
                    printf(
                        "== %s, A:0x%0*lX, C:0x%0*lX%s ==\r\n",
                        irda_get_protocol_name(message->protocol),
                        irda_get_protocol_address_length(message->protocol),
                        message->address,
                        irda_get_protocol_command_length(message->protocol),
                        message->command,
                        message->repeat ? " R" : "");
                } else {
                    printf("== unknown data ==\r\n");
                    snprintf(
                        irda_monitor->display_text,
                        sizeof(irda_monitor->display_text),
                        "unknown data");
                    view_port_update(view_port);
                }
                printf("{");
                while(print_counter != counter) {
                    printf("%lu, ", delays->timing[print_counter].duration);
                    ++print_counter;
                    if(print_counter >= IRDA_TIMINGS_SIZE) {
                        print_counter = 0;
                    }
                }
                printf("\r\n};\r\n");
            }
        }
    }

    api_hal_irda_rx_irq_deinit();
    irda_free_decoder(irda_monitor->handler);
    osMessageQueueDelete(irda_monitor->event_queue);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close("notification");
    furi_record_close("gui");
    free(irda_monitor);

    return 0;
}
