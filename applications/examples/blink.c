#include "flipper.h"
#include "flipper_v2.h"

void application_blink(void* p) {
    // create pin
    GpioPin led = {.pin = GPIO_PIN_8, .port = GPIOA};

    // TODO open record
    GpioPin* led_record = &led;

    // configure pin
    pinMode(led_record, GpioModeOutputOpenDrain);

    while(1) {
        digitalWrite(led_record, HIGH);
        delay(500);
        digitalWrite(led_record, LOW);
        delay(500);
    }
}