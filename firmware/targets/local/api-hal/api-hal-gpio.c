#include "api-hal-gpio.h"
#include <stdio.h>

// init GPIO
void hal_gpio_init(GpioPin* gpio, GpioMode mode, GpioPull pull, GpioSpeed speed){
    // TODO more mode
    if(gpio->pin != 0) {
        switch(mode) {
        case GpioModeInput:
            printf("[GPIO] %s%d input\n", gpio->port, gpio->pin);
            break;

        case GpioModeOutputPushPull:
            printf("[GPIO] %s%d push pull\n", gpio->port, gpio->pin);
            break;

        case GpioModeOutputOpenDrain:
            printf("[GPIO] %s%d open drain\n", gpio->port, gpio->pin);
            break;
        
        default:
            printf("[GPIO] %s%d mode %d unsupported\n", gpio->port, gpio->pin, mode);
            break;
        }
    } else {
        printf("[GPIO] no pin\n");
    }
}

// write value to GPIO, false = LOW, true = HIGH
void hal_gpio_write(GpioPin* gpio, bool state){
    if(gpio->pin != 0) {
        if(state) {
            printf("[GPIO] %s%d on\n", gpio->port, gpio->pin);
        } else {
            printf("[GPIO] %s%d off\n", gpio->port, gpio->pin);
        }
    } else {
        printf("[GPIO] no pin\n");
    }
}

// read value from GPIO, false = LOW, true = HIGH
bool hal_gpio_read(GpioPin* gpio){
    // TODO emulate pin state?
    return false;
}
