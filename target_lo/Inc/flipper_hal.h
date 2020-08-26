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
    const char* port;
    uint32_t pin;
    GpioMode mode;
} GpioPin;

void app_gpio_init(GpioPin gpio, GpioMode mode);

inline void app_gpio_write(GpioPin gpio, bool state) {
    if(gpio.pin != 0) {
        if(state) {
            printf("[GPIO] %s%d on\n", gpio.port, gpio.pin);
        } else {
            printf("[GPIO] %s%d off\n", gpio.port, gpio.pin);
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

#define GPIOA "PA"
#define GPIOB "PB"
#define GPIOC "PC"
#define GPIOD "PD"
#define GPIOE "PE"

#define GPIO_PIN_0 0
#define GPIO_PIN_1 1
#define GPIO_PIN_2 2
#define GPIO_PIN_3 3
#define GPIO_PIN_4 4
#define GPIO_PIN_5 5
#define GPIO_PIN_6 6
#define GPIO_PIN_7 7
#define GPIO_PIN_8 8
#define GPIO_PIN_9 9
#define GPIO_PIN_10 10
#define GPIO_PIN_11 11
#define GPIO_PIN_12 12
#define GPIO_PIN_13 13
#define GPIO_PIN_14 14
#define GPIO_PIN_15 15
