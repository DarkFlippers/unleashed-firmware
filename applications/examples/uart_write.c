#include "flipper.h"
#include <string.h>
#include "log.h"
#include "flipper_v2.h"

void application_uart_write(void* p) {
    // Red led for showing progress
    GpioPin led = {.pin = GPIO_PIN_8, .port = GPIOA};
    // TODO open record
    GpioPin* led_record = &led;

    pinMode(led_record, GpioModeOutputOpenDrain);

    // get_default_log open "tty" record
    FuriRecordSubscriber* log = get_default_log();

    // create buffer
    const char test_string[] = "test\n";
    furi_write(log, test_string, strlen(test_string));

    // for example, create counter and show its value
    uint8_t counter = 0;

    while(1) {
        // continously write it to UART
        printf("counter: %d\n", counter);
        counter++;

        // flash at every send
        digitalWrite(led_record, LOW);
        delay(50);
        digitalWrite(led_record, HIGH);

        // delay with overall perion of 1s
        delay(950);
    }
}