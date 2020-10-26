#pragma once
#include "main.h"
#include "stdbool.h"

void hal_pwm_set(float value, float freq, TIM_HandleTypeDef* tim, uint32_t channel);
void hal_pwmn_set(float value, float freq, TIM_HandleTypeDef* tim, uint32_t channel);
void hal_pwm_stop(TIM_HandleTypeDef* tim, uint32_t channel);
void hal_pwmn_stop(TIM_HandleTypeDef* tim, uint32_t channel);
