#include "api-hal-interrupt.h"

#include <furi.h>
#include <main.h>
#include <stm32wbxx_ll_tim.h>

volatile ApiHalInterruptISR api_hal_tim_tim2_isr = NULL;

#define API_HAL_INTERRUPT_DMA_COUNT 2
#define API_HAL_INTERRUPT_DMA_CHANNELS_COUNT 8

volatile ApiHalInterruptISR api_hal_dma_channel_isr[API_HAL_INTERRUPT_DMA_COUNT][API_HAL_INTERRUPT_DMA_CHANNELS_COUNT] = {0};

void api_hal_interrupt_init() {
    NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    FURI_LOG_I("FuriHalInterrupt", "Init OK");
}

void api_hal_interrupt_set_timer_isr(TIM_TypeDef* timer, ApiHalInterruptISR isr) {
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

void api_hal_interrupt_set_dma_channel_isr(DMA_TypeDef* dma, uint32_t channel, ApiHalInterruptISR isr) {
    --channel; // Pascal 
    furi_check(dma);
    furi_check(channel < API_HAL_INTERRUPT_DMA_CHANNELS_COUNT);
    if (dma == DMA1) {
        api_hal_dma_channel_isr[0][channel] = isr;
    } else if (dma == DMA1) {
        api_hal_dma_channel_isr[1][channel] = isr;
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

/* Timer 2 */
void TIM2_IRQHandler(void) {
    if (api_hal_tim_tim2_isr) {
        api_hal_tim_tim2_isr();
    } else {
        HAL_TIM_IRQHandler(&htim2);
    }
}

/* DMA 1 */
void DMA1_Channel1_IRQHandler(void) {
    if (api_hal_dma_channel_isr[0][0]) api_hal_dma_channel_isr[0][0]();
}

void DMA1_Channel2_IRQHandler(void) {
    if (api_hal_dma_channel_isr[0][1]) api_hal_dma_channel_isr[0][1]();
}

void DMA1_Channel3_IRQHandler(void) {
    if (api_hal_dma_channel_isr[0][2]) api_hal_dma_channel_isr[0][2]();
}

void DMA1_Channel4_IRQHandler(void) {
    if (api_hal_dma_channel_isr[0][3]) api_hal_dma_channel_isr[0][3]();
}

void DMA1_Channel5_IRQHandler(void) {
    if (api_hal_dma_channel_isr[0][4]) api_hal_dma_channel_isr[0][4]();
}

void DMA1_Channel6_IRQHandler(void) {
    if (api_hal_dma_channel_isr[0][5]) api_hal_dma_channel_isr[0][5]();
}

void DMA1_Channel7_IRQHandler(void) {
    if (api_hal_dma_channel_isr[0][6]) api_hal_dma_channel_isr[0][6]();
}

void DMA1_Channel8_IRQHandler(void) {
    if (api_hal_dma_channel_isr[0][7]) api_hal_dma_channel_isr[0][7]();
}

/* DMA 2 */
void DMA2_Channel1_IRQHandler(void) {
    if (api_hal_dma_channel_isr[1][0]) api_hal_dma_channel_isr[1][0]();
}

void DMA2_Channel2_IRQHandler(void) {
    if (api_hal_dma_channel_isr[1][1]) api_hal_dma_channel_isr[1][1]();
}

void DMA2_Channel3_IRQHandler(void) {
    if (api_hal_dma_channel_isr[1][2]) api_hal_dma_channel_isr[1][2]();
}

void DMA2_Channel4_IRQHandler(void) {
    if (api_hal_dma_channel_isr[1][3]) api_hal_dma_channel_isr[1][3]();
}

void DMA2_Channel5_IRQHandler(void) {
    if (api_hal_dma_channel_isr[1][4]) api_hal_dma_channel_isr[1][4]();
}

void DMA2_Channel6_IRQHandler(void) {
    if (api_hal_dma_channel_isr[1][5]) api_hal_dma_channel_isr[1][5]();
}

void DMA2_Channel7_IRQHandler(void) {
    if (api_hal_dma_channel_isr[1][6]) api_hal_dma_channel_isr[1][6]();
}

void DMA2_Channel8_IRQHandler(void) {
    if (api_hal_dma_channel_isr[1][7]) api_hal_dma_channel_isr[1][7]();
}
