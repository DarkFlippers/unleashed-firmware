#include <furi-hal-vibro.h>
#include <furi-hal-gpio.h>

#define TAG "FuriHalVibro"

void furi_hal_vibro_init() {
    hal_gpio_init(&vibro_gpio, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&vibro_gpio, false);
    FURI_LOG_I(TAG, "Init OK");

}

void furi_hal_vibro_on(bool value) {
    hal_gpio_write(&vibro_gpio, value);
}
