#include "flipper.h"

void application_blink(void* p) {
    // create pin
    GpioPin led = {.pin = GPIO_PIN_8, .port = GPIOA};
    
    // configure pin
    pinMode(led, GpioModeOutput);

    while(1) {
        digitalWrite(led, HIGH);
        delay(500);
        digitalWrite(led, LOW);
        delay(500);
    }
}