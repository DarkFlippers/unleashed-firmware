#include "../gpio_app_i.h"
#include <furi_hal_gpio.h>

static I2CScannerState* i2c_scanner_state;

void gpio_scene_i2c_scanner_ok_callback(InputType type, void* context) {
    furi_assert(context);
    GpioApp* app = context;

    if(type == InputTypeRelease) {
        notification_message(app->notifications, &sequence_set_green_255);
        gpio_i2c_scanner_run_once(i2c_scanner_state);
        notification_message(app->notifications, &sequence_reset_green);
        gpio_i2c_scanner_update_state(app->gpio_i2c_scanner, i2c_scanner_state);
    }
}

void gpio_scene_i2c_scanner_on_enter(void* context) {
    GpioApp* app = context;
    i2c_scanner_state = malloc(sizeof(I2CScannerState));

    gpio_i2c_scanner_set_ok_callback(
        app->gpio_i2c_scanner, gpio_scene_i2c_scanner_ok_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, GpioAppViewI2CScanner);
}

bool gpio_scene_i2c_scanner_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void gpio_scene_i2c_scanner_on_exit(void* context) {
    UNUSED(context);
    free(i2c_scanner_state);
}
