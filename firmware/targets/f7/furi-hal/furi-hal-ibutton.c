#include <furi-hal-ibutton.h>
#include <furi-hal-resources.h>

void furi_hal_ibutton_start() {
    furi_hal_ibutton_pin_high();
    hal_gpio_init(&ibutton_gpio, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
}

void furi_hal_ibutton_stop() {
    furi_hal_ibutton_pin_high();
    hal_gpio_init(&ibutton_gpio, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void furi_hal_ibutton_pin_low() {
    hal_gpio_write(&ibutton_gpio, false);
}

void furi_hal_ibutton_pin_high() {
    hal_gpio_write(&ibutton_gpio, true);
}

bool furi_hal_ibutton_pin_get_level() {
    return hal_gpio_read(&ibutton_gpio);
}
