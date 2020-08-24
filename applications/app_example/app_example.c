#include <stdio.h>
#include "flipper.h"
#include "debug.h"

void furi_widget(void* param);
void furi_test_app(void* param);
void furi_next_test_app(void* param);

/*
widget simply print ping message
*/
void furi_widget(void* param) {
    FILE* debug_uart = get_debug();

    fprintf(debug_uart, "start furi widget: %s\n", (char*)param);

    while(1) {
        fprintf(debug_uart, "furi widget\n");
        delay(10);
    }
}

/*
it simply start, then start child widget, wait about 1 sec (with ping evey 200 ms),
kill the widget, continue with 500 ms ping.
*/
void furi_test_app(void* param) {

    uint8_t cnt = 0;

    while(1) {
        fprintf(debug_uart, "furi test app %d\n", cnt);
        delay(10);

        if(cnt == 2) {
            fprintf(debug_uart, "go to next app\n");
            furiac_switch(furi_next_test_app, "next_test", NULL);
            fprintf(debug_uart, "unsuccessful switch\n");
            while(1) {
                delay(1000);
            }
        }

        cnt++;
    }
}

void furi_next_test_app(void* param) {
    FILE* debug_uart = get_debug();

    fprintf(debug_uart, "start next test app\n");

    delay(10);

    fprintf(debug_uart, "exit next app\n");
    furiac_exit(NULL);

    while(1) {
        // this code must not be called
        fprintf(debug_uart, "next app: something went wrong\n");
        delay(10);
    }
}

/*
FILE* debug_uart = get_debug();

fprintf(debug_uart, "hello Flipper!\n");

GpioPin red_led = {LED_RED_GPIO_Port, LED_RED_Pin};

app_gpio_init(red_led, GpioModeOutput);


while(1) {
    delay(100);
    app_gpio_write(red_led, true);
    delay(100);
    app_gpio_write(red_led, false);
}
*/