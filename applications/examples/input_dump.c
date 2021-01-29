#include <furi.h>
#include <stdio.h>
#include <input/input.h>

typedef union {
    unsigned int packed;
    InputState state;
} InputDump;

static void state_cb(const void* value, void* ctx) {
    InputDump dump = {.packed = 0};
    dump.state = *(InputState*)value;

    printf("state: %02x\r\n", dump.packed);
}

static void event_cb(const void* value, void* ctx) {
    const InputEvent* event = value;

    printf("event: %02x %s\r\n", event->input, event->state ? "pressed" : "released");
}

void application_input_dump(void* p) {
    // open record
    ValueManager* state_record = furi_record_open("input_state");
    subscribe_pubsub(&state_record->pubsub, state_cb, NULL);

    PubSub* event_record = furi_record_open("input_events");
    subscribe_pubsub(event_record, event_cb, NULL);

    printf("Example app [input dump]\r\n");

    for(;;) {
        delay(100);
    }
}
