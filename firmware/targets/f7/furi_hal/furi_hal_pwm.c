#include "furi_hal_pwm.h"
#include <core/check.h>
#include <furi_hal_resources.h>

#include <stdint.h>
#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_lptim.h>
#include <stm32wbxx_ll_rcc.h>

#include <furi.h>

const uint32_t lptim_psc_table[] = {
    LL_LPTIM_PRESCALER_DIV1,
    LL_LPTIM_PRESCALER_DIV2,
    LL_LPTIM_PRESCALER_DIV4,
    LL_LPTIM_PRESCALER_DIV8,
    LL_LPTIM_PRESCALER_DIV16,
    LL_LPTIM_PRESCALER_DIV32,
    LL_LPTIM_PRESCALER_DIV64,
    LL_LPTIM_PRESCALER_DIV128,
};

void furi_hal_pwm_start(FuriHalPwmOutputId channel, uint32_t freq, uint8_t duty) {
    if(channel == FuriHalPwmOutputIdTim1PA7) {
        furi_hal_gpio_init_ex(
            &gpio_ext_pa7,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn1TIM1);

        FURI_CRITICAL_ENTER();
        LL_TIM_DeInit(TIM1);
        FURI_CRITICAL_EXIT();

        LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);
        LL_TIM_SetRepetitionCounter(TIM1, 0);
        LL_TIM_SetClockDivision(TIM1, LL_TIM_CLOCKDIVISION_DIV1);
        LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
        LL_TIM_EnableARRPreload(TIM1);

        LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
        LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
        LL_TIM_OC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH1N, LL_TIM_OCPOLARITY_HIGH);
        LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH1);
        LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1N);

        LL_TIM_EnableAllOutputs(TIM1);

        furi_hal_pwm_set_params(channel, freq, duty);

        LL_TIM_EnableCounter(TIM1);
    } else if(channel == FuriHalPwmOutputIdLptim2PA4) {
        furi_hal_gpio_init_ex(
            &gpio_ext_pa4,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn14LPTIM2);

        FURI_CRITICAL_ENTER();
        LL_LPTIM_DeInit(LPTIM2);
        FURI_CRITICAL_EXIT();

        LL_LPTIM_SetUpdateMode(LPTIM2, LL_LPTIM_UPDATE_MODE_ENDOFPERIOD);
        LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM2_CLKSOURCE_PCLK1);
        LL_LPTIM_SetClockSource(LPTIM2, LL_LPTIM_CLK_SOURCE_INTERNAL);
        LL_LPTIM_ConfigOutput(
            LPTIM2, LL_LPTIM_OUTPUT_WAVEFORM_PWM, LL_LPTIM_OUTPUT_POLARITY_INVERSE);
        LL_LPTIM_SetCounterMode(LPTIM2, LL_LPTIM_COUNTER_MODE_INTERNAL);

        LL_LPTIM_Enable(LPTIM2);

        furi_hal_pwm_set_params(channel, freq, duty);

        LL_LPTIM_StartCounter(LPTIM2, LL_LPTIM_OPERATING_MODE_CONTINUOUS);
    }
}

void furi_hal_pwm_stop(FuriHalPwmOutputId channel) {
    if(channel == FuriHalPwmOutputIdTim1PA7) {
        furi_hal_gpio_init_simple(&gpio_ext_pa7, GpioModeAnalog);
        FURI_CRITICAL_ENTER();
        LL_TIM_DeInit(TIM1);
        FURI_CRITICAL_EXIT();
    } else if(channel == FuriHalPwmOutputIdLptim2PA4) {
        furi_hal_gpio_init_simple(&gpio_ext_pa4, GpioModeAnalog);
        FURI_CRITICAL_ENTER();
        LL_LPTIM_DeInit(LPTIM2);
        FURI_CRITICAL_EXIT();
    }
}

void furi_hal_pwm_set_params(FuriHalPwmOutputId channel, uint32_t freq, uint8_t duty) {
    furi_assert(freq > 0);
    uint32_t freq_div = 64000000LU / freq;

    if(channel == FuriHalPwmOutputIdTim1PA7) {
        uint32_t prescaler = freq_div / 0x10000LU;
        uint32_t period = freq_div / (prescaler + 1);
        uint32_t compare = period * duty / 100;

        LL_TIM_SetPrescaler(TIM1, prescaler);
        LL_TIM_SetAutoReload(TIM1, period - 1);
        LL_TIM_OC_SetCompareCH1(TIM1, compare);
    } else if(channel == FuriHalPwmOutputIdLptim2PA4) {
        uint32_t prescaler = 0;
        uint32_t period = 0;

        bool clock_lse = false;

        do {
            period = freq_div / (1 << prescaler);
            if(period <= 0xFFFF) {
                break;
            }
            prescaler++;
            if(prescaler > 7) {
                prescaler = 0;
                clock_lse = true;
                period = 32768LU / freq;
                break;
            }
        } while(1);

        uint32_t compare = period * duty / 100;

        LL_LPTIM_SetPrescaler(LPTIM2, lptim_psc_table[prescaler]);
        LL_LPTIM_SetAutoReload(LPTIM2, period);
        LL_LPTIM_SetCompare(LPTIM2, compare);

        if(clock_lse) {
            LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM2_CLKSOURCE_LSE);
        } else {
            LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM2_CLKSOURCE_PCLK1);
        }
    }
}
