#include "flipper.h"
#include "flipper_v2.h"

void application_blink(void* p) {
    // create pin
    GpioPin led = led_gpio[0];

    // TODO open record
    GpioPin* led_record = &led;

    // configure pin
    gpio_init(led_record, GpioModeOutputOpenDrain);

    while(1) {
        gpio_write(led_record, true);
        delay(500);
        gpio_write(led_record, false);
        delay(500);
    }
}