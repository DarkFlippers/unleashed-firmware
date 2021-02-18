#include <furi.h>
#include <api-hal.h>

#include <input/input.h>

typedef struct {
    GpioPin* vibro;
} Ctx;

static void button_handler(const void* value, void* _ctx) {
    const InputEvent* event = value;
    Ctx* ctx = (Ctx*)_ctx;

    if(event->key != InputKeyOk) return;

    if(event->type == InputTypePress) {
        api_hal_light_set(LightGreen, 0xFF);
        gpio_write(ctx->vibro, true);
    } else if(event->type == InputTypeRelease) {
        api_hal_light_set(LightGreen, 0x00);
        gpio_write(ctx->vibro, false);
    }
}

int32_t application_vibro(void* p) {
    Ctx ctx = {.vibro = (GpioPin*)&vibro_gpio};

    gpio_init(ctx.vibro, GpioModeOutputPushPull);
    gpio_write(ctx.vibro, false);

    // subscribe on buttons
    PubSub* event_record = furi_record_open("input_events");
    furi_check(event_record);
    subscribe_pubsub(event_record, button_handler, &ctx);

    while(1) {
        osDelay(osWaitForever);
    }

    return 0;
}