#include "gui_event.h"

#include <flipper_v2.h>
#include <assert.h>

#define GUI_EVENT_MQUEUE_SIZE 8

struct GuiEvent {
    PubSub* input_event_record;
    osMessageQueueId_t mqueue;
};

void gui_event_input_events_callback(const void* value, void* ctx) {
    assert(ctx);
    GuiEvent* gui_event = ctx;

    GuiMessage message;
    message.type = GuiMessageTypeInput;
    message.input = *(InputEvent*)value;

    osMessageQueuePut(gui_event->mqueue, &message, 0, osWaitForever);
}

GuiEvent* gui_event_alloc() {
    GuiEvent* gui_event = furi_alloc(sizeof(GuiEvent));
    // Allocate message que
    gui_event->mqueue = osMessageQueueNew(GUI_EVENT_MQUEUE_SIZE, sizeof(GuiMessage), NULL);
    assert(gui_event->mqueue);

    // Input
    gui_event->input_event_record = furi_open("input_events");
    assert(gui_event->input_event_record != NULL);
    subscribe_pubsub(gui_event->input_event_record, gui_event_input_events_callback, gui_event);

    return gui_event;
}

void gui_event_free(GuiEvent* gui_event) {
    assert(gui_event);
    assert(osMessageQueueDelete(gui_event->mqueue) == osOK);
    free(gui_event);
}

void gui_event_messsage_send(GuiEvent* gui_event, GuiMessage* message) {
    assert(gui_event);
    assert(message);
    osMessageQueuePut(gui_event->mqueue, message, 0, 0);
}

GuiMessage gui_event_message_next(GuiEvent* gui_event) {
    assert(gui_event);
    GuiMessage message;

    assert(osMessageQueueGet(gui_event->mqueue, &message, NULL, osWaitForever) == osOK);

    return message;
}
