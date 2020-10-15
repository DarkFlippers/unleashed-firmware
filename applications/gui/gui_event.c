#include "gui_event.h"

#include <flipper.h>
#include <assert.h>

#define GUI_EVENT_MQUEUE_SIZE 8

struct GuiEvent {
    FuriRecordSubscriber* input_event_record;
    osMessageQueueId_t mqueue;
    osMutexId_t lock_mutex;
};

void gui_event_input_events_callback(const void* value, size_t size, void* ctx) {
    assert(ctx);
    GuiEvent* gui_event = ctx;

    GuiMessage message;
    message.type = GuiMessageTypeInput;
    message.input = *(InputEvent*)value;

    osMessageQueuePut(gui_event->mqueue, &message, 0, 0);
}

GuiEvent* gui_event_alloc() {
    GuiEvent* gui_event = furi_alloc(sizeof(GuiEvent));
    // Allocate message que
    gui_event->mqueue = osMessageQueueNew(GUI_EVENT_MQUEUE_SIZE, sizeof(GuiMessage), NULL);
    assert(gui_event->mqueue);

    // Input
    gui_event->input_event_record = furi_open_deprecated(
        "input_events", false, false, gui_event_input_events_callback, NULL, gui_event);
    assert(gui_event->input_event_record != NULL);
    // Lock mutex
    gui_event->lock_mutex = osMutexNew(NULL);
    assert(gui_event->lock_mutex);
    gui_event_lock(gui_event);

    return gui_event;
}

void gui_event_free(GuiEvent* gui_event) {
    assert(gui_event);
    gui_event_unlock(gui_event);
    assert(osMessageQueueDelete(gui_event->mqueue) == osOK);
    free(gui_event);
}

void gui_event_lock(GuiEvent* gui_event) {
    assert(gui_event);
    assert(osMutexAcquire(gui_event->lock_mutex, osWaitForever) == osOK);
}

void gui_event_unlock(GuiEvent* gui_event) {
    assert(gui_event);
    assert(osMutexRelease(gui_event->lock_mutex) == osOK);
}

void gui_event_messsage_send(GuiEvent* gui_event, GuiMessage* message) {
    assert(gui_event);
    assert(message);
    osMessageQueuePut(gui_event->mqueue, message, 0, 0);
}

GuiMessage gui_event_message_next(GuiEvent* gui_event) {
    assert(gui_event);
    GuiMessage message;
    gui_event_unlock(gui_event);
    while(osMessageQueueGet(gui_event->mqueue, &message, NULL, osWaitForever) != osOK) {
    };
    gui_event_lock(gui_event);
    return message;
}
