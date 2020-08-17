/*
Flipper devices inc.

GPIO and HAL implementations
*/

#include "main.h"
#include "flipper_hal.h"
#include <stdio.h>

void app_gpio_init(GpioPin gpio, GpioMode mode) {
    if(gpio.pin != 0) {

        switch(mode) {
            case GpioModeInput:
                printf("[GPIO] %d:%d input\n", gpio.port, gpio.pin);
            break;

            case GpioModeOutput: 
                printf("[GPIO] %d:%d push pull\n", gpio.port, gpio.pin);
            break;

            case GpioModeOpenDrain:
                printf("[GPIO] %d:%d open drain\n", gpio.port, gpio.pin);
            break;
        }

        gpio.mode = mode;
    } else {
        printf("[GPIO] no pin\n");
    }
}

void delay_us(uint32_t time) {
    // How to deal with it
    printf("[DELAY] %d us\n", time);
}

void pwm_set(float value, float freq, TIM_HandleTypeDef* tim, uint32_t channel) {
    printf("[TIM] set pwm %d:%d %f Hz, %f%%\n", *tim, channel, freq, value * 100.);
}