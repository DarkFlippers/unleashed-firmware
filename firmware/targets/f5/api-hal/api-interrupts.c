#include "api-hal/api-interrupt-mgr.h"
#include <main.h>

extern void api_interrupt_call(InterruptType type, void* hw);

/* interrupts */

/* Comparator trigger event */
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef* hcomp) {
    api_interrupt_call(InterruptTypeComparatorTrigger, hcomp);
}

/* Timer input capture event */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim) {
    api_interrupt_call(InterruptTypeTimerCapture, htim);
}

/* Output compare event */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim) {
    api_interrupt_call(InterruptTypeTimerOutputCompare, htim);
}

/* Timer update event */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    api_interrupt_call(InterruptTypeTimerUpdate, htim);
}
