#include <furi_hal_vibro.h>
#include <furi_hal_gpio.h>

#define TAG "FuriHalVibro"

void furi_hal_vibro_init(void) {
    furi_hal_gpio_init(&gpio_vibro, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_vibro, false);
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_vibro_on(bool value) {
    furi_hal_gpio_write(&gpio_vibro, value);
}
