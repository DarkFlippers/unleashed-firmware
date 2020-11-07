#include "flipper_v2.h"

static void event_cb(const void* value, void* ctx) {
    const InputEvent* event = value;

    uint32_t* delay_time = acquire_mutex(ctx, 0);
    if(delay_time == NULL) return;

    if(event->input == InputUp && *delay_time < 1000) {
        *delay_time += 5;
    }

    if(event->input == InputDown && *delay_time > 10) {
        *delay_time -= 5;
    }
    release_mutex(ctx, delay_time);
}

void application_strobe(void* p) {
    // WAT
    osDelay(100);

    // create pins
    GpioPin red = {.pin = LED_RED_Pin, .port = LED_RED_GPIO_Port};
    GpioPin green = {.pin = LED_GREEN_Pin, .port = LED_GREEN_GPIO_Port};
    GpioPin blue = {.pin = LED_BLUE_Pin, .port = LED_BLUE_GPIO_Port};

    GpioPin* red_record = &red;
    GpioPin* green_record = &green;
    GpioPin* blue_record = &blue;

    // configure pins
    gpio_init(red_record, GpioModeOutputOpenDrain);
    gpio_init(green_record, GpioModeOutputOpenDrain);
    gpio_init(blue_record, GpioModeOutputOpenDrain);

    uint32_t delay_time_holder = 100;
    ValueMutex delay_mutex;
    init_mutex(&delay_mutex, &delay_time_holder, sizeof(delay_time_holder));

    PubSub* event_record = furi_open("input_events");
    furi_check(event_record);
    subscribe_pubsub(event_record, event_cb, &delay_mutex);

    while(1) {
        uint32_t delay_time = 100;
        read_mutex_block(&delay_mutex, &delay_time, sizeof(delay_time));

        gpio_write(red_record, false);
        gpio_write(green_record, false);
        gpio_write(blue_record, false);
        osDelay(delay_time / 10);
        gpio_write(red_record, true);
        gpio_write(green_record, true);
        gpio_write(blue_record, true);
        osDelay(delay_time);
    }
}