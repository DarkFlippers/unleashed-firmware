#include <furi-hal-vibro.h>
#include <furi-hal-gpio.h>

void furi_hal_vibro_init() {
    hal_gpio_init(&vibro_gpio, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&vibro_gpio, false);
    FURI_LOG_I("FuriHalVibro", "Init OK");

}

void furi_hal_vibro_on(bool value) {
    hal_gpio_write(&vibro_gpio, value);
}
