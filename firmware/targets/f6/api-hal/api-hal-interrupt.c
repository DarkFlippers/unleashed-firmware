#include "api-hal-interrupt.h"

#include <furi.h>
#include <main.h>
#include <stm32wbxx_ll_tim.h>

volatile ApiHalInterruptISR api_hal_tim_tim2_isr = NULL;

void TIM2_IRQHandler(void) {
    if (api_hal_tim_tim2_isr) {
        api_hal_tim_tim2_isr();
    } else {
        HAL_TIM_IRQHandler(&htim2);
    }
}

void api_hal_interrupt_set_timer_isr(TIM_TypeDef *timer, ApiHalInterruptISR isr) {
    if (timer == TIM2) {
        if (isr) {
            furi_assert(api_hal_tim_tim2_isr == NULL);
        } else {
            furi_assert(api_hal_tim_tim2_isr != NULL);
        }
        api_hal_tim_tim2_isr = isr;
    } else {
        furi_check(0);
    }
}

extern void api_interrupt_call(InterruptType type, void* hw);

/* ST HAL symbols */

/* Comparator trigger event */
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef* hcomp) {
    api_interrupt_call(InterruptTypeComparatorTrigger, hcomp);
}

/* Timer update event */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    api_interrupt_call(InterruptTypeTimerUpdate, htim);
}
