#include "flipper_v2.h"

static void event_cb(const void* value, void* ctx) {
    xSemaphoreGive((SemaphoreHandle_t*)ctx);
}

const uint32_t BACKLIGHT_TIME = 10000;

void backlight_control(void* p) {
    // TODO use FURI
    HAL_GPIO_WritePin(DISPLAY_BACKLIGHT_GPIO_Port, DISPLAY_BACKLIGHT_Pin, GPIO_PIN_SET);

    StaticSemaphore_t event_descriptor;
    SemaphoreHandle_t update = xSemaphoreCreateCountingStatic(255, 0, &event_descriptor);

    // open record
    PubSub* event_record = furi_open("input_events");
    assert(event_record != NULL);
    subscribe_pubsub(event_record, event_cb, (void*)update);

    // we ready to work
    furiac_ready();

    while(1) {
        // wait for event
        if(xSemaphoreTake(update, BACKLIGHT_TIME) == pdTRUE) {
            HAL_GPIO_WritePin(DISPLAY_BACKLIGHT_GPIO_Port, DISPLAY_BACKLIGHT_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(DISPLAY_BACKLIGHT_GPIO_Port, DISPLAY_BACKLIGHT_Pin, GPIO_PIN_RESET);
        }
    }
}