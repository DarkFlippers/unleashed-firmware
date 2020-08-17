/*
Flipper devices inc.

GPIO and HAL implementations
*/

#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "main.h"

typedef enum {
    GpioModeInput,
    GpioModeOutput,
    GpioModeOpenDrain
} GpioMode;

typedef struct {
    uint32_t port;
    uint32_t pin;
    GpioMode mode;
} GpioPin;

void app_gpio_init(GpioPin gpio, GpioMode mode);

inline void app_gpio_write(GpioPin gpio, bool state) {
    if(gpio.pin != 0) {
        if(state) {
            printf("[GPIO] %d:%d on\n", gpio.port, gpio.pin);
        } else {
            printf("[GPIO] %d:%d off\n", gpio.port, gpio.pin);
        }
    } else {
        printf("[GPIO] no pin\n");
    }
}

inline bool app_gpio_read(GpioPin gpio) {
    // TODO emulate pin state?

    return false;
}

void delay_us(uint32_t time);

void pwm_set(float value, float freq, TIM_HandleTypeDef* tim, uint32_t channel);

extern TIM_HandleTypeDef htim8;

inline void app_tim_ic_init(bool both) {
    printf("[TIM] init\n");
}

inline void app_tim_pulse(uint32_t width) {
    printf("[TIM] pulse %d\n", width);
}

inline void app_tim_stop() {
    printf("[TIM] stop\n");
}