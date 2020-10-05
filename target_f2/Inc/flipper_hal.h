/*
Flipper devices inc.

GPIO and HAL implementations
*/

#pragma once

#include <stdbool.h>
#include "main.h"

typedef enum { GpioModeInput, GpioModeOutput, GpioModeOpenDrain } GpioMode;

typedef struct {
    GPIO_TypeDef* port;
    uint32_t pin;
} GpioPin;

void app_gpio_init(GpioPin gpio, GpioMode mode);

static inline void app_gpio_write(GpioPin gpio, bool state) {
    if(gpio.pin != 0) {
        if(state) {
            gpio.port->BSRR = (uint32_t)gpio.pin;
        } else {
            gpio.port->BRR = (uint32_t)gpio.pin;
        }
    }
}

static inline bool app_gpio_read(GpioPin gpio) {
    if(gpio.pin != 0) {
        return (gpio.port->IDR & gpio.pin) != 0x00u;
    }

    return false;
}

void delay_us(uint32_t time);

void pwm_set(float value, float freq, TIM_HandleTypeDef* tim, uint32_t channel);

extern TIM_HandleTypeDef htim8;

static inline void app_tim_ic_init(bool both) {
    HAL_TIM_OC_Stop(&htim8, TIM_CHANNEL_2);

    TIM_IC_InitTypeDef sConfigIC = {0};
    sConfigIC.ICPolarity = both ? TIM_INPUTCHANNELPOLARITY_BOTHEDGE :
                                  TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim8, &sConfigIC, TIM_CHANNEL_2);

    HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2);
}

static inline void app_tim_pulse(uint32_t width) {
    htim8.State = HAL_TIM_STATE_BUSY;

    __HAL_TIM_DISABLE(&htim8);

    __HAL_TIM_SET_COUNTER(&htim8, 0);

    TIM_OC_InitTypeDef sConfigOC;
    sConfigOC.OCMode = TIM_OCMODE_INACTIVE;
    sConfigOC.Pulse = (uint16_t)(width);
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    // HAL_TIM_OC_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_2);

    htim8.Lock = HAL_LOCKED;

    /* Configure the TIM Channel 2 in Output Compare */
    TIM_OC2_SetConfig(htim8.Instance, &sConfigOC);

    htim8.Lock = HAL_UNLOCKED;

    // TIM_CCxChannelCmd(htim8.Instance, TIM_CHANNEL_2, TIM_CCx_ENABLE);

    /* Reset the CCxE Bit */
    htim8.Instance->CCER &= ~(TIM_CCER_CC1E << (TIM_CHANNEL_2 & 0x1FU));

    /* Set or reset the CCxE Bit */
    htim8.Instance->CCER |= (uint32_t)(TIM_CCx_ENABLE << (TIM_CHANNEL_2 & 0x1FU));

    __HAL_TIM_MOE_ENABLE(&htim8);
    __HAL_TIM_ENABLE(&htim8);

    htim8.State = HAL_TIM_STATE_READY;
}

static inline void app_tim_stop() {
    HAL_TIM_OC_Stop(&htim8, TIM_CHANNEL_2);
    HAL_TIM_IC_Stop(&htim8, TIM_CHANNEL_2);
}
