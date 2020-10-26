#include <stdio.h>
#include "flipper.h"
#include "flipper_v2.h"
#include "log.h"

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
    pinMode(red_record, GpioModeOutputOpenDrain);
    pinMode(green_record, GpioModeOutputOpenDrain);
    pinMode(blue_record, GpioModeOutputOpenDrain);

    digitalWrite(red_record, HIGH);
    digitalWrite(green_record, HIGH);
    digitalWrite(blue_record, LOW);

    uint32_t exitcode = run_minunit();

    if(exitcode == 0) {
        // test passed
        digitalWrite(red_record, HIGH);
        digitalWrite(green_record, LOW);
        digitalWrite(blue_record, HIGH);
    } else {
        // test failed
        digitalWrite(red_record, LOW);
        digitalWrite(green_record, HIGH);
        digitalWrite(blue_record, HIGH);
    }

    set_exitcode(exitcode);

    furiac_exit(NULL);
}