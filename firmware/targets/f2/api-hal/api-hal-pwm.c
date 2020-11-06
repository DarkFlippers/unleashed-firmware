#include "api-hal-pwm.h"

void hal_pwm_set(float value, float freq, TIM_HandleTypeDef* tim, uint32_t channel) {
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

void hal_pwmn_set(float value, float freq, TIM_HandleTypeDef* tim, uint32_t channel) {
    tim->Init.CounterMode = TIM_COUNTERMODE_UP;
    tim->Init.Period = (uint32_t)((SystemCoreClock / (tim->Init.Prescaler + 1)) / freq - 1);
    tim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(tim);

    TIM_OC_InitTypeDef sConfigOC;

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = (uint16_t)(tim->Init.Period * value);
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(tim, &sConfigOC, channel);
    HAL_TIMEx_PWMN_Start(tim, channel);
}

void hal_pwm_stop(TIM_HandleTypeDef* tim, uint32_t channel) {
    HAL_TIM_PWM_Stop(tim, channel);
}

void hal_pwmn_stop(TIM_HandleTypeDef* tim, uint32_t channel) {
    HAL_TIMEx_PWMN_Stop(tim, channel);
}

void irda_pwm_set(float value, float freq){
    hal_pwm_set(value, freq, &IRDA_TIM, IRDA_CH);
}

void irda_pwm_stop(){
    hal_pwm_stop(&IRDA_TIM, IRDA_CH);
}