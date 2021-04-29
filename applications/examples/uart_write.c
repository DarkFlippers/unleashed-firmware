#include <furi.h>
#include <api-hal.h>
#include <string.h>

int32_t application_uart_write(void* p) {
    // Red led for showing progress
    GpioPin led = {.pin = GPIO_PIN_8, .port = GPIOA};
    // TODO open record
    GpioPin* led_record = &led;

    hal_gpio_init(led_record, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);

    // create buffer
    const char test_string[] = "test\n";
    printf(test_string);

    // for example, create counter and show its value
    uint8_t counter = 0;

    while(1) {
        // continously write it to UART
        printf("counter: %d\n", counter);
        counter++;

        // flash at every send
        hal_gpio_write(led_record, false);
        delay(50);
        hal_gpio_write(led_record, true);

        // delay with overall perion of 1s
        delay(950);
    }

    return 0;
}