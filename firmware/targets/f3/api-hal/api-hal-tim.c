#include "cmsis_os.h"
#include "api-hal-tim.h"

void tim_irda_rx_init(void) {
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
}