#include "api-hal/api-interrupt-mgr.h"
#include <main.h>

extern void api_interrupt_call(InterruptType type, void* hw);

/* interrupts */
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef* hcomp) {
    api_interrupt_call(InterruptTypeComparatorTrigger, hcomp);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim) {
    api_interrupt_call(InterruptTypeTimerCapture, htim);
}