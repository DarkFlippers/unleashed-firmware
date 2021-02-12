#include <furi.h>

#define BACKLIGHT_TIME 10000
#define BACKLIGHT_FLAG_ACTIVITY 0x00000001U

static void event_cb(const void* value, void* ctx) {
    osThreadFlagsSet((osThreadId_t)ctx, BACKLIGHT_FLAG_ACTIVITY);
}

int32_t backlight_control(void* p) {
    // TODO open record
    const GpioPin* backlight_record = &backlight_gpio;

    // configure pin
    gpio_init(backlight_record, GpioModeOutputPushPull);
    gpio_write(backlight_record, true);

    // open record
    PubSub* event_record = furi_record_open("input_events");
    subscribe_pubsub(event_record, event_cb, (void*)osThreadGetId());

    while(1) {
        // wait for event
        if(osThreadFlagsWait(BACKLIGHT_FLAG_ACTIVITY, osFlagsWaitAny, BACKLIGHT_TIME) ==
           BACKLIGHT_FLAG_ACTIVITY) {
            gpio_write(backlight_record, true);
        } else {
            gpio_write(backlight_record, false);
        }
    }

    return 0;
}