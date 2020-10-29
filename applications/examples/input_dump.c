#include "flipper_v2.h"
#include <stdio.h>

static void state_cb(const void* value, void* ctx) {
    const InputState* state = value;

    printf("state: %02x\n", *state);
}

static void event_cb(const void* value, void* ctx) {
    const InputEvent* event = value;

    printf("event: %02x %s\n", event->input, event->state ? "pressed" : "released");
}

void application_input_dump(void* p) {
    // open record
    ValueManager* state_record = furi_open("input_state");
    furi_check(state_record);
    subscribe_pubsub(&state_record->pubsub, state_cb, NULL);

    PubSub* event_record = furi_open("input_events");
    furi_check(event_record);
    subscribe_pubsub(event_record, event_cb, NULL);

    for(;;) {
        delay(100);
    }
}
