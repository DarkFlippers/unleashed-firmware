#include "furi-hal-interrupt.h"

#include <furi.h>
#include <main.h>
#include <stm32wbxx_ll_tim.h>

#define TAG "FuriHalInterrupt"

volatile FuriHalInterruptISR furi_hal_tim_tim2_isr = NULL;
volatile FuriHalInterruptISR furi_hal_tim_tim1_isr = NULL;

#define FURI_HAL_INTERRUPT_DMA_COUNT 2
#define FURI_HAL_INTERRUPT_DMA_CHANNELS_COUNT 8

volatile FuriHalInterruptISR furi_hal_dma_channel_isr[FURI_HAL_INTERRUPT_DMA_COUNT][FURI_HAL_INTERRUPT_DMA_CHANNELS_COUNT] = {0};

void furi_hal_interrupt_init() {
    NVIC_SetPriority(RCC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(RCC_IRQn);

    NVIC_SetPriority(TAMP_STAMP_LSECSS_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(TAMP_STAMP_LSECSS_IRQn);

    NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_interrupt_set_timer_isr(TIM_TypeDef* timer, FuriHalInterruptISR isr) {
    if (timer == TIM2) {
        if (isr) {
            furi_assert(furi_hal_tim_tim2_isr == NULL);
        } else {
            furi_assert(furi_hal_tim_tim2_isr != NULL);
        }
        furi_hal_tim_tim2_isr = isr;
    } else if (timer == TIM1) {
        if (isr) {
            furi_assert(furi_hal_tim_tim1_isr == NULL);
        } else {
            furi_assert(furi_hal_tim_tim1_isr != NULL);
        }
        furi_hal_tim_tim1_isr = isr;
    } else {
        furi_crash(NULL);
    }
}

void furi_hal_interrupt_set_dma_channel_isr(DMA_TypeDef* dma, uint32_t channel, FuriHalInterruptISR isr) {
    --channel; // Pascal
    furi_check(dma);
    furi_check(channel < FURI_HAL_INTERRUPT_DMA_CHANNELS_COUNT);
    if (dma == DMA1) {
        furi_hal_dma_channel_isr[0][channel] = isr;
    } else if (dma == DMA2) {
        furi_hal_dma_channel_isr[1][channel] = isr;
    } else {
        furi_crash(NULL);
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
    if (furi_hal_tim_tim2_isr) {
        furi_hal_tim_tim2_isr();
    } else {
        HAL_TIM_IRQHandler(&htim2);
    }
}

/* Timer 1 Update */
void TIM1_UP_TIM16_IRQHandler(void) {
    if (furi_hal_tim_tim1_isr) {
        furi_hal_tim_tim1_isr();
    } else {
        HAL_TIM_IRQHandler(&htim1);
    }
}

/* DMA 1 */
void DMA1_Channel1_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[0][0]) furi_hal_dma_channel_isr[0][0]();
}

void DMA1_Channel2_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[0][1]) furi_hal_dma_channel_isr[0][1]();
}

void DMA1_Channel3_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[0][2]) furi_hal_dma_channel_isr[0][2]();
}

void DMA1_Channel4_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[0][3]) furi_hal_dma_channel_isr[0][3]();
}

void DMA1_Channel5_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[0][4]) furi_hal_dma_channel_isr[0][4]();
}

void DMA1_Channel6_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[0][5]) furi_hal_dma_channel_isr[0][5]();
}

void DMA1_Channel7_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[0][6]) furi_hal_dma_channel_isr[0][6]();
}

void DMA1_Channel8_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[0][7]) furi_hal_dma_channel_isr[0][7]();
}

/* DMA 2 */
void DMA2_Channel1_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[1][0]) furi_hal_dma_channel_isr[1][0]();
}

void DMA2_Channel2_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[1][1]) furi_hal_dma_channel_isr[1][1]();
}

void DMA2_Channel3_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[1][2]) furi_hal_dma_channel_isr[1][2]();
}

void DMA2_Channel4_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[1][3]) furi_hal_dma_channel_isr[1][3]();
}

void DMA2_Channel5_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[1][4]) furi_hal_dma_channel_isr[1][4]();
}

void DMA2_Channel6_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[1][5]) furi_hal_dma_channel_isr[1][5]();
}

void DMA2_Channel7_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[1][6]) furi_hal_dma_channel_isr[1][6]();
}

void DMA2_Channel8_IRQHandler(void) {
    if (furi_hal_dma_channel_isr[1][7]) furi_hal_dma_channel_isr[1][7]();
}


void TAMP_STAMP_LSECSS_IRQHandler(void) {
    if (LL_RCC_IsActiveFlag_LSECSS()) {
        LL_RCC_ClearFlag_LSECSS();
        if (!LL_RCC_LSE_IsReady()) {
            FURI_LOG_E(TAG, "LSE CSS fired: resetting system");
            NVIC_SystemReset();
        } else {
            FURI_LOG_E(TAG, "LSE CSS fired: but LSE is alive");
        }
    }
}

void RCC_IRQHandler(void) {
}


void NMI_Handler(void) {
    if (LL_RCC_IsActiveFlag_HSECSS()) {
        LL_RCC_ClearFlag_HSECSS();
        FURI_LOG_E(TAG, "HSE CSS fired: resetting system");
        NVIC_SystemReset();
    }
}

void HardFault_Handler(void) {
    if ((*(volatile uint32_t *)CoreDebug_BASE) & (1 << 0)) {
        __asm("bkpt 1");
    }
    while (1) {}
}

void MemManage_Handler(void) {
    __asm("bkpt 1");
    while (1) {}
}

void BusFault_Handler(void) {
    __asm("bkpt 1");
    while (1) {}
}

void UsageFault_Handler(void) {
    __asm("bkpt 1");
    while (1) {}
}

void DebugMon_Handler(void) {

}
