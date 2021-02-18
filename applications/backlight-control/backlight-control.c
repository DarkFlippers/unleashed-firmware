#include <furi.h>
#include <api-hal.h>

#define BACKLIGHT_TIME 30000
#define BACKLIGHT_FLAG_ACTIVITY 0x00000001U

static void event_cb(const void* value, void* ctx) {
    osThreadFlagsSet((osThreadId_t)ctx, BACKLIGHT_FLAG_ACTIVITY);
}

int32_t backlight_control(void* p) {
    // open record
    PubSub* event_record = furi_record_open("input_events");
    subscribe_pubsub(event_record, event_cb, (void*)osThreadGetId());

    api_hal_light_set(LightBacklight, 0xFF);

    while(1) {
        // wait for event
        if(osThreadFlagsWait(BACKLIGHT_FLAG_ACTIVITY, osFlagsWaitAny, BACKLIGHT_TIME) ==
           BACKLIGHT_FLAG_ACTIVITY) {
            api_hal_light_set(LightBacklight, 0xFF);
        } else {
            api_hal_light_set(LightBacklight, 0x00);
        }
    }

    return 0;
}