#include "flipper_v2.h"

static void event_cb(const void* value, void* ctx) {
    xSemaphoreGive((SemaphoreHandle_t*)ctx);
}

const uint32_t BACKLIGHT_TIME = 10000;

void backlight_control(void* p) {
    // create pin
    GpioPin backlight = backlight_gpio;

    // TODO open record
    GpioPin* backlight_record = &backlight;

    // configure pin
    gpio_init(backlight_record, GpioModeOutputPushPull);
    gpio_write(backlight_record, true);

    StaticSemaphore_t event_descriptor;
    SemaphoreHandle_t update = xSemaphoreCreateCountingStatic(255, 0, &event_descriptor);

    // open record
    PubSub* event_record = furi_open("input_events");
    furi_check(event_record);
    subscribe_pubsub(event_record, event_cb, (void*)update);

    // we ready to work
    furiac_ready();

    while(1) {
        // wait for event
        if(xSemaphoreTake(update, BACKLIGHT_TIME) == pdTRUE) {
            gpio_write(backlight_record, true);
        } else {
            gpio_write(backlight_record, false);
        }
    }
}