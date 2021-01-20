#include <stdio.h>
#include <furi.h>

// #include "flipper-core.h" TODO: Rust build disabled

int run_minunit();

void flipper_test_app(void* p) {
    // create pins
    GpioPin red = {.pin = LED_RED_Pin, .port = LED_RED_GPIO_Port};
    GpioPin green = {.pin = LED_GREEN_Pin, .port = LED_GREEN_GPIO_Port};
    GpioPin blue = {.pin = LED_BLUE_Pin, .port = LED_BLUE_GPIO_Port};

    GpioPin* red_record = &red;
    GpioPin* green_record = &green;
    GpioPin* blue_record = &blue;

    // configure pins
    gpio_init(red_record, GpioModeOutputOpenDrain);
    gpio_init(green_record, GpioModeOutputOpenDrain);
    gpio_init(blue_record, GpioModeOutputOpenDrain);

    gpio_write(red_record, true);
    gpio_write(green_record, true);
    gpio_write(blue_record, false);

    uint32_t exitcode = run_minunit();

    if(exitcode == 0) {
        // test passed
        gpio_write(red_record, true);
        gpio_write(green_record, false);
        gpio_write(blue_record, true);
    } else {
        // test failed
        gpio_write(red_record, false);
        gpio_write(green_record, true);
        gpio_write(blue_record, true);
    }

    set_exitcode(exitcode);

    furiac_exit(NULL);
}