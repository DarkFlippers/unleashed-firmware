#include "furi/common_defines.h"
#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>

#include <notification/notification_messages.h>

typedef enum {
    BlinkEventTypeTick,
    BlinkEventTypeInput,
} BlinkEventType;

typedef struct {
    BlinkEventType type;
    InputEvent input;
} BlinkEvent;

static const NotificationSequence blink_test_sequence_hw_blink_start_red = {
    &message_blink_start_10,
    &message_blink_set_color_red,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence blink_test_sequence_hw_blink_green = {
    &message_blink_set_color_green,
    NULL,
};

static const NotificationSequence blink_test_sequence_hw_blink_blue = {
    &message_blink_set_color_blue,
    NULL,
};

static const NotificationSequence blink_test_sequence_hw_blink_stop = {
    &message_blink_stop,
    NULL,
};

static const NotificationSequence* blink_test_colors[] = {
    &sequence_blink_red_100,
    &sequence_blink_green_100,
    &sequence_blink_blue_100,
    &sequence_blink_yellow_100,
    &sequence_blink_cyan_100,
    &sequence_blink_magenta_100,
    &sequence_blink_white_100,
    &blink_test_sequence_hw_blink_start_red,
    &blink_test_sequence_hw_blink_green,
    &blink_test_sequence_hw_blink_blue,
    &blink_test_sequence_hw_blink_stop,
};

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

    uint8_t state = 0;
    BlinkEvent event;

    while(1) {
        furi_check(osMessageQueueGet(event_queue, &event, NULL, osWaitForever) == osOK);
        if(event.type == BlinkEventTypeInput) {
            if((event.input.type == InputTypeShort) && (event.input.key == InputKeyBack)) {
                break;
            }
        } else {
            notification_message(notifications, blink_test_colors[state]);
            state++;
            if(state >= COUNT_OF(blink_test_colors)) {
                state = 0;
            }
        }
    }

    notification_message(notifications, &blink_test_sequence_hw_blink_stop);

    osTimerDelete(timer);

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);

    furi_record_close("notification");
    furi_record_close("gui");

    return 0;
}
