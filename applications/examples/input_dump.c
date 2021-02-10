#include <furi.h>
#include <stdio.h>
#include <input/input.h>

typedef union {
    unsigned int packed;
    InputType state;
} InputDump;

static void event_cb(const void* value, void* ctx) {
    const InputEvent* event = value;

    printf("event: %02x %s\r\n", event->key, event->type ? "pressed" : "released");
}

void application_input_dump(void* p) {
    // open record
    PubSub* event_record = furi_record_open("input_events");
    subscribe_pubsub(event_record, event_cb, NULL);

    printf("Example app [input dump]\r\n");

    for(;;) {
        delay(100);
    }
}
