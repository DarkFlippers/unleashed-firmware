#include "api-hal-irda.h"
#include <cmsis_os2.h>
#include <api-hal-interrupt.h>
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

static void api_hal_irda_handle_timeout(void) {
    /* Timers CNT register starts to counting from 0 to ARR, but it is
     * reseted when Channel 1 catches interrupt. It is not reseted by
     * channel 2, though, so we have to distract it's values (see TimerIRQSourceCCI1 ISR).
     * This can cause false timeout: when time is over, but we started
     * receiving new signal few microseconds ago, because CNT register
     * is reseted once per period, not per sample. */
    if (LL_GPIO_IsInputPinSet(gpio_irda_rx.port, gpio_irda_rx.pin) == 0)
        return;

    if (timer_irda.timeout_callback)
        timer_irda.timeout_callback(timer_irda.timeout_context);
}

/* High pin level is a Space state of IRDA signal. Invert level for further processing. */
static void api_hal_irda_handle_capture(TimerIRQSource source) {
    uint32_t duration = 0;
    bool level = 0;

    switch (source) {
    case TimerIRQSourceCCI1:
        duration = LL_TIM_IC_GetCaptureCH1(TIM2) - LL_TIM_IC_GetCaptureCH2(TIM2);
        level = 1;
        break;
    case TimerIRQSourceCCI2:
        duration = LL_TIM_IC_GetCaptureCH2(TIM2);
        level = 0;
        break;
    default:
        furi_check(0);
    }

    if (timer_irda.capture_callback)
        timer_irda.capture_callback(timer_irda.capture_context, level, duration);
}

static void api_hal_irda_isr() {
    if(LL_TIM_IsActiveFlag_CC3(TIM2)) {
        LL_TIM_ClearFlag_CC3(TIM2);
        api_hal_irda_handle_timeout();
    }
    if(LL_TIM_IsActiveFlag_CC1(TIM2)) {
        LL_TIM_ClearFlag_CC1(TIM2);

        if(READ_BIT(TIM2->CCMR1, TIM_CCMR1_CC1S)) {
            // input capture
            api_hal_irda_handle_capture(TimerIRQSourceCCI1);
        }
    }
    if(LL_TIM_IsActiveFlag_CC2(TIM2)) {
        LL_TIM_ClearFlag_CC2(TIM2);

        if(READ_BIT(TIM2->CCMR1, TIM_CCMR1_CC2S)) {
            // input capture
            api_hal_irda_handle_capture(TimerIRQSourceCCI2);
        }
    }
}

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

    api_hal_interrupt_set_timer_isr(TIM2, api_hal_irda_isr);

    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableCounter(TIM2);
}

void api_hal_irda_rx_irq_deinit(void) {
    LL_TIM_DeInit(TIM2);
    api_hal_interrupt_set_timer_isr(TIM2, NULL);
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
