#include "api-hal-irda.h"
#include <cmsis_os2.h>
#include <api-hal-resources.h>

#include <stdint.h>
#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_gpio.h>

#include <stdio.h>
#include <furi.h>
#include <main.h>
#include <api-hal-pwm.h>

static struct{
    ApiHalIrdaCaptureCallback capture_callback;
    void *capture_context;
    ApiHalIrdaTimeoutCallback timeout_callback;
    void *timeout_context;
} timer_irda;

typedef enum{
    TimerIRQSourceCCI1,
    TimerIRQSourceCCI2,
} TimerIRQSource;

void api_hal_irda_rx_irq_init(void) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

    hal_gpio_init_ex(&gpio_irda_rx, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn1TIM2);

    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = 64 - 1;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 0x7FFFFFFE;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM2, &TIM_InitStruct);

    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableARRPreload(TIM2);
    LL_TIM_SetTriggerInput(TIM2, LL_TIM_TS_TI1FP1);
    LL_TIM_SetSlaveMode(TIM2, LL_TIM_SLAVEMODE_RESET);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);
    LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_FALLING);
    LL_TIM_DisableIT_TRIG(TIM2);
    LL_TIM_DisableDMAReq_TRIG(TIM2);
    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
    LL_TIM_EnableMasterSlaveMode(TIM2);
    LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);
    LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_RISING);
    LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_INDIRECTTI);
    LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);

    LL_TIM_EnableIT_CC1(TIM2);
    LL_TIM_EnableIT_CC2(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableCounter(TIM2);

    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(TIM2_IRQn);
}

/* Doesn't work. F5 deprecated. */
void api_hal_irda_rx_irq_deinit(void) {
    LL_TIM_DeInit(TIM2);
}

void api_hal_irda_rx_timeout_irq_init(uint32_t timeout_ms) {
    LL_TIM_OC_SetCompareCH3(TIM2, timeout_ms * 1000);
    LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_ACTIVE);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH3);
    LL_TIM_EnableIT_CC3(TIM2);
}

bool api_hal_irda_rx_irq_is_busy(void) {
    return (LL_TIM_IsEnabledIT_CC1(TIM2) || LL_TIM_IsEnabledIT_CC2(TIM2));
}

void api_hal_irda_rx_irq_set_callback(ApiHalIrdaCaptureCallback callback, void *ctx) {
    timer_irda.capture_callback = callback;
    timer_irda.capture_context = ctx;
}

void api_hal_irda_rx_timeout_irq_set_callback(ApiHalIrdaTimeoutCallback callback, void *ctx) {
    timer_irda.timeout_callback = callback;
    timer_irda.timeout_context = ctx;
}

void api_hal_irda_pwm_set(float value, float freq) {
    hal_pwmn_set(value, freq, &IRDA_TX_TIM, IRDA_TX_CH);
}

void api_hal_irda_pwm_stop() {
    hal_pwmn_stop(&IRDA_TX_TIM, IRDA_TX_CH);
}
