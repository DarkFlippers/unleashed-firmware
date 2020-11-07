#include "flipper.h"
#include "flipper_v2.h"

void rgb_set(bool r, bool g, bool b, GpioPin* led_r, GpioPin* led_g, GpioPin* led_b) {
    gpio_write(led_r, !r);
    gpio_write(led_g, !g);
    gpio_write(led_b, !b);
}

void application_blink(void* p) {
    // create pin
    GpioPin led_r = led_gpio[0];
    GpioPin led_g = led_gpio[1];
    GpioPin led_b = led_gpio[2];

    // TODO open record
    GpioPin* led_r_record = &led_r;
    GpioPin* led_g_record = &led_g;
    GpioPin* led_b_record = &led_b;

    // configure pin
    gpio_init(led_r_record, GpioModeOutputOpenDrain);
    gpio_init(led_g_record, GpioModeOutputOpenDrain);
    gpio_init(led_b_record, GpioModeOutputOpenDrain);

    while(1) {
        rgb_set(1, 0, 0, led_r_record, led_g_record, led_b_record);
        delay(500);
        rgb_set(0, 1, 0, led_r_record, led_g_record, led_b_record);
        delay(500);
        rgb_set(1, 1, 0, led_r_record, led_g_record, led_b_record);
        delay(500);
        rgb_set(0, 0, 1, led_r_record, led_g_record, led_b_record);
        delay(500);
        rgb_set(1, 0, 1, led_r_record, led_g_record, led_b_record);
        delay(500);
        rgb_set(0, 1, 1, led_r_record, led_g_record, led_b_record);
        delay(500);
        rgb_set(1, 1, 1, led_r_record, led_g_record, led_b_record);
        delay(500);
        rgb_set(0, 0, 0, led_r_record, led_g_record, led_b_record);
        delay(500);
    }
}