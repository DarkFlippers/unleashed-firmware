#include <stdio.h>
#include "flipper.h"
#include "log.h"

// #include "flipper-core.h" TODO: Rust build disabled

int run_minunit();

void flipper_test_app(void* p) {
    // create pins
    GpioPin red = {.pin = LED_RED_Pin, .port = LED_RED_GPIO_Port};
    GpioPin green = {.pin = LED_GREEN_Pin, .port = LED_GREEN_GPIO_Port};
    GpioPin blue = {.pin = LED_BLUE_Pin, .port = LED_BLUE_GPIO_Port};

    // configure pins
    pinMode(red, GpioModeOpenDrain);
    pinMode(green, GpioModeOpenDrain);
    pinMode(blue, GpioModeOpenDrain);

    digitalWrite(red, HIGH);
    digitalWrite(green, HIGH);
    digitalWrite(blue, LOW);

    uint32_t exitcode = run_minunit();

    if(exitcode == 0) {
        // test passed
        digitalWrite(red, HIGH);
        digitalWrite(green, LOW);
        digitalWrite(blue, HIGH);
    } else {
        // test failed
        digitalWrite(red, LOW);
        digitalWrite(green, HIGH);
        digitalWrite(blue, HIGH);
    }

    set_exitcode(exitcode);

    furiac_exit(NULL);
}