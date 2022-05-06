#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>

#include <notification/notification_messages.h>

#define BLINK_COLOR_COUNT 7

typedef enum {
    BlinkEventTypeTick,
    BlinkEventTypeInput,
} BlinkEventType;

typedef struct {
    BlinkEventType type;
    InputEvent input;
} BlinkEvent;

static void blink_test_update(void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;
    BlinkEvent event = {.type = BlinkEventTypeTick};
    // It's OK to loose this event if system overloaded
    osMessageQueuePut(event_queue, &event, 0, 0);
}

static void blink_test_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Blink application");
}

static void blink_test_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;

    BlinkEvent event = {.type = BlinkEventTypeInput, .input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

int32_t blink_test_app(void* p) {
    UNUSED(p);
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(BlinkEvent), NULL);

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, blink_test_draw_callback, NULL);
    view_port_input_callback_set(view_port, blink_test_input_callback, event_queue);
    osTimerId_t timer = osTimerNew(blink_test_update, osTimerPeriodic, event_queue, NULL);
    osTimerStart(timer, osKernelGetTickFreq());

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
        if(event.type == BlinkEventTypeInput) {
            if((event.input.type == InputTypeShort) && (event.input.key == InputKeyBack)) {
                break;
            }
        } else {
            notification_message(notifications, colors[state]);
            state++;
            if(state >= BLINK_COLOR_COUNT) {
                state = 0;
            }
        }
    }

    osTimerDelete(timer);

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);

    furi_record_close("notification");
    furi_record_close("gui");

    return 0;
}
