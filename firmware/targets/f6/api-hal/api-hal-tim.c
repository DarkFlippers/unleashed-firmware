#include "api-hal-tim_i.h"
#include "api-hal-irda_i.h"
#include <stm32wbxx_ll_tim.h>
#include <furi.h>

void TIM2_IRQHandler(void) {
    bool consumed = false;

    if(LL_TIM_IsActiveFlag_CC1(TIM2) == 1) {
        if(LL_TIM_IsEnabledIT_CC1(TIM2)) {
            LL_TIM_ClearFlag_CC1(TIM2);

            if(READ_BIT(TIM2->CCMR1, TIM_CCMR1_CC1S)) {
                // input capture
                api_hal_irda_tim_isr(TimerIRQSourceCCI1);
                consumed = true;
            } else {
                // output compare
                //  HAL_TIM_OC_DelayElapsedCallback(htim);
                //  HAL_TIM_PWM_PulseFinishedCallback(htim);
            }
        }
    }
    if(LL_TIM_IsActiveFlag_CC2(TIM2) == 1) {
        if(LL_TIM_IsEnabledIT_CC2(TIM2)) {
            LL_TIM_ClearFlag_CC2(TIM2);

            if(READ_BIT(TIM2->CCMR1, TIM_CCMR1_CC2S)) {
                // input capture
                api_hal_irda_tim_isr(TimerIRQSourceCCI2);
                consumed = true;
            } else {
                // output compare
                //  HAL_TIM_OC_DelayElapsedCallback(htim);
                //  HAL_TIM_PWM_PulseFinishedCallback(htim);
            }
        }
    }

    // TODO move all timers on LL hal
    if(!consumed) {
        // currently backed up with a crutch, we need more bicycles
        HAL_TIM_IRQHandler(&htim2);
    }
}
