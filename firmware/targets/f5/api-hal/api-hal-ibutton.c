#include <api-hal-ibutton.h>
#include <api-hal-resources.h>

void api_hal_ibutton_start() {
    api_hal_ibutton_pin_high();
    hal_gpio_init(&ibutton_gpio, GpioModeOutputOpenDrain, GpioSpeedLow, GpioPullNo);
}

void api_hal_ibutton_stop() {
    api_hal_ibutton_pin_high();
    hal_gpio_init(&ibutton_gpio, GpioModeAnalog, GpioSpeedLow, GpioPullNo);
}

void api_hal_ibutton_pin_low() {
    hal_gpio_write(&ibutton_gpio, false);
}

void api_hal_ibutton_pin_high() {
    hal_gpio_write(&ibutton_gpio, true);
}

bool api_hal_ibutton_pin_get_level() {
    return hal_gpio_read(&ibutton_gpio);
}
