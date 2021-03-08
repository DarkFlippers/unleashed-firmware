#include <api-hal-vibro.h>
#include <api-hal-gpio.h>

void api_hal_vibro_init() {
    hal_gpio_init(&vibro_gpio, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&vibro_gpio, false);
}

void api_hal_vibro_on(bool value) {
    hal_gpio_write(&vibro_gpio, value);
}
