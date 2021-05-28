#include <furi.h>
#include <api-hal.h>

#include <gui/gui.h>
#include <input/input.h>

#include <notification/notification-messages.h>

#define BLINK_COLOR_COUNT 7

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} BlinkEvent;

void blink_update(void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;

    BlinkEvent event = {.type = EventTypeTick};
    osMessageQueuePut(event_queue, &event, 0, 0);
}

void blink_draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Blink application");
}

void blink_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;

    BlinkEvent event = {.type = EventTypeKey, .input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, 0);
}

int32_t application_blink(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(BlinkEvent), NULL);

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    furi_check(view_port);
    view_port_draw_callback_set(view_port, blink_draw_callback, NULL);
    view_port_input_callback_set(view_port, blink_input_callback, event_queue);
    osTimerId_t timer = osTimerNew(blink_update, osTimerPeriodic, event_queue, NULL);
    osTimerStart(timer, 1000);

    // Register view port in GUI
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    NotificationApp* notifications = furi_record_open("notification");

    const NotificationSequence* colors[BLINK_COLOR_COUNT] = {
        &sequence_blink_red_100,
        &sequence_blink_green_100,
        &sequence_blink_blue_100,
        &sequence_blink_yellow_100,
        &sequence_blink_cyan_100,
        &sequence_blink_magenta_100,
        &sequence_blink_white_100,
    };

    uint8_t state = 0;
    BlinkEvent event;

    while(1) {
        furi_check(osMessageQueueGet(event_queue, &event, NULL, osWaitForever) == osOK);
        if(event.type == EventTypeKey) {
            if((event.input.type == InputTypeShort) && (event.input.key == InputKeyBack)) {
                furi_record_close("notification");
                view_port_enabled_set(view_port, false);
                gui_remove_view_port(gui, view_port);
                view_port_free(view_port);
                osMessageQueueDelete(event_queue);
                osTimerDelete(timer);

                return 0;
            }
        } else {
            notification_message(notifications, colors[state]);

            state++;
            if(state >= BLINK_COLOR_COUNT) {
                state = 0;
            }
        }
    }

    return 0;
}