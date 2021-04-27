#include <furi.h>
#include <api-hal.h>
#include <input/input.h>

static void event_cb(const void* value, void* ctx) {
    const InputEvent* event = value;

    uint32_t* delay_time = acquire_mutex(ctx, 0);
    if(delay_time == NULL) return;

    if(event->key == InputKeyUp && *delay_time < 1000) {
        *delay_time += 5;
    }

    if(event->key == InputKeyDown && *delay_time > 10) {
        *delay_time -= 5;
    }
    release_mutex(ctx, delay_time);
}

void application_strobe(void* p) {
    // WAT
    osDelay(100);

    uint32_t delay_time_holder = 100;
    ValueMutex delay_mutex;
    init_mutex(&delay_mutex, &delay_time_holder, sizeof(delay_time_holder));

    PubSub* event_record = furi_record_open("input_events");
    subscribe_pubsub(event_record, event_cb, &delay_mutex);

    while(1) {
        uint32_t delay_time = 100;
        read_mutex_block(&delay_mutex, &delay_time, sizeof(delay_time));

        api_hal_light_set(LightRed, 0x00);
        api_hal_light_set(LightGreen, 0x00);
        api_hal_light_set(LightBlue, 0x00);
        osDelay(delay_time / 10);

        api_hal_light_set(LightRed, 0xFF);
        api_hal_light_set(LightGreen, 0xFF);
        api_hal_light_set(LightBlue, 0xFF);
        osDelay(delay_time);
    }
}