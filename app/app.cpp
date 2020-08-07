#include "Arduino.h"

extern "C" void app() {
    printf("hello Flipper!\n");

    GpioPin red_led = {LED_RED_GPIO_Port, LED_RED_Pin};

    app_gpio_init(red_led, GpioModeOutput);

    while(1) {
        delay(100);
        app_gpio_write(red_led, true);
        delay(100);
        app_gpio_write(red_led, false);
    }
}