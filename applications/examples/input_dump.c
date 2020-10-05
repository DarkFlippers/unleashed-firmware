#include "flipper.h"
#include <stdio.h>

static void state_cb(const void* value, size_t size, void* ctx) {
    const InputState* state = value;

    printf("state: %02x\n", *state);
}

static void event_cb(const void* value, size_t size, void* ctx) {
    const InputEvent* event = value;

    printf("event: %02x %s\n", event->input, event->state ? "pressed" : "released");
}

void application_input_dump(void* p) {
    // open record
    FuriRecordSubscriber* state_record =
        furi_open("input_state", false, false, state_cb, NULL, NULL);
    FuriRecordSubscriber* event_record =
        furi_open("input_events", false, false, event_cb, NULL, NULL);

    for(;;) {
        delay(100);
    }
}
