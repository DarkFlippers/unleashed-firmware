/*
Flipper devices inc.

GPIO and HAL implementations
*/

#include "main.h"
#include "flipper_hal.h"

void app_gpio_init(GpioPin gpio, GpioMode mode) {
    if(gpio.pin != 0) {
        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.Pin = gpio.pin;
        GPIO_InitStruct.Pull = GPIO_NOPULL;

        switch(mode) {
        case GpioModeInput:
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            break;

        case GpioModeOutput:
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
            break;

        case GpioModeOpenDrain:
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
            break;
        }

        HAL_GPIO_Init(gpio.port, &GPIO_InitStruct);
    }
}

void delay_us_init_DWT(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;
}

void delay_us(float time) {
    uint32_t start = DWT->CYCCNT;
    uint32_t time_ticks = time * (SystemCoreClock / 1000000);
    while((DWT->CYCCNT - start) < time_ticks) {
    };
}

void pwm_set(float value, float freq, TIM_HandleTypeDef* tim, uint32_t channel) {
    tim->Init.CounterMode = TIM_COUNTERMODE_UP;
    tim->Init.Period = (uint32_t)((SystemCoreClock / (tim->Init.Prescaler + 1)) / freq);
    tim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(tim);

    TIM_OC_InitTypeDef sConfigOC;

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = (uint16_t)(tim->Init.Period * value);
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(tim, &sConfigOC, channel);
    HAL_TIM_PWM_Start(tim, channel);
}

void pwm_stop(TIM_HandleTypeDef* tim, uint32_t channel) {
    HAL_TIM_PWM_Stop(tim, channel);
}