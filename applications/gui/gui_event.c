#include "gui_event.h"

#include <flipper_v2.h>

#define GUI_EVENT_MQUEUE_SIZE 8

struct GuiEvent {
    PubSub* input_event_record;
    osTimerId_t timer;
    osMessageQueueId_t mqueue;
};

void gui_event_timer_callback(void* arg) {
    assert(arg);
    GuiEvent* gui_event = arg;

    GuiMessage message;
    message.type = GuiMessageTypeRedraw;

    osMessageQueuePut(gui_event->mqueue, &message, 0, osWaitForever);
}

void gui_event_input_events_callback(const void* value, void* ctx) {
    furi_assert(value);
    furi_assert(ctx);

    GuiEvent* gui_event = ctx;

    GuiMessage message;
    message.type = GuiMessageTypeInput;
    message.input = *(InputEvent*)value;

    osMessageQueuePut(gui_event->mqueue, &message, 0, osWaitForever);
}

GuiEvent* gui_event_alloc() {
    GuiEvent* gui_event = furi_alloc(sizeof(GuiEvent));

    // Allocate message queue
    gui_event->mqueue = osMessageQueueNew(GUI_EVENT_MQUEUE_SIZE, sizeof(GuiMessage), NULL);
    furi_check(gui_event->mqueue);

    gui_event->timer = osTimerNew(gui_event_timer_callback, osTimerPeriodic, gui_event, NULL);
    assert(gui_event->timer);
    osTimerStart(gui_event->timer, 1000 / 10);

    // Input
    gui_event->input_event_record = furi_open("input_events");
    furi_check(gui_event->input_event_record != NULL);
    subscribe_pubsub(gui_event->input_event_record, gui_event_input_events_callback, gui_event);

    return gui_event;
}

void gui_event_free(GuiEvent* gui_event) {
    furi_assert(gui_event);
    furi_check(osMessageQueueDelete(gui_event->mqueue) == osOK);
    free(gui_event);
}

void gui_event_messsage_send(GuiEvent* gui_event, GuiMessage* message) {
    furi_assert(gui_event);
    furi_assert(message);
    osMessageQueuePut(gui_event->mqueue, message, 0, 0);
}

GuiMessage gui_event_message_next(GuiEvent* gui_event) {
    furi_assert(gui_event);
    GuiMessage message;

    furi_check(osMessageQueueGet(gui_event->mqueue, &message, NULL, osWaitForever) == osOK);

    return message;
}
