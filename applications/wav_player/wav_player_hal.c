#include "wav_player_hal.h"
#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_dma.h>

#define FURI_HAL_SPEAKER_TIMER TIM16
#define FURI_HAL_SPEAKER_CHANNEL LL_TIM_CHANNEL_CH1
#define DMA_INSTANCE DMA1, LL_DMA_CHANNEL_1

void wav_player_speaker_init() {
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = 4;
    TIM_InitStruct.Autoreload = 255;
    LL_TIM_Init(FURI_HAL_SPEAKER_TIMER, &TIM_InitStruct);

    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
    TIM_OC_InitStruct.CompareValue = 127;
    LL_TIM_OC_Init(FURI_HAL_SPEAKER_TIMER, FURI_HAL_SPEAKER_CHANNEL, &TIM_OC_InitStruct);
}

void wav_player_speaker_start() {
    LL_TIM_EnableAllOutputs(FURI_HAL_SPEAKER_TIMER);
    LL_TIM_EnableCounter(FURI_HAL_SPEAKER_TIMER);
}

void wav_player_speaker_stop() {
    LL_TIM_DisableAllOutputs(FURI_HAL_SPEAKER_TIMER);
    LL_TIM_DisableCounter(FURI_HAL_SPEAKER_TIMER);
}

void wav_player_dma_init(uint32_t address, size_t size) {
    uint32_t dma_dst = (uint32_t) & (FURI_HAL_SPEAKER_TIMER->CCR1);

    LL_DMA_ConfigAddresses(DMA_INSTANCE, address, dma_dst, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(DMA_INSTANCE, size);

    LL_DMA_SetPeriphRequest(DMA_INSTANCE, LL_DMAMUX_REQ_TIM16_UP);
    LL_DMA_SetDataTransferDirection(DMA_INSTANCE, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(DMA_INSTANCE, LL_DMA_PRIORITY_VERYHIGH);
    LL_DMA_SetMode(DMA_INSTANCE, LL_DMA_MODE_CIRCULAR);
    LL_DMA_SetPeriphIncMode(DMA_INSTANCE, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA_INSTANCE, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA_INSTANCE, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(DMA_INSTANCE, LL_DMA_MDATAALIGN_HALFWORD);

    LL_DMA_EnableIT_TC(DMA_INSTANCE);
    LL_DMA_EnableIT_HT(DMA_INSTANCE);
}

void wav_player_dma_start() {
    LL_DMA_EnableChannel(DMA_INSTANCE);
    LL_TIM_EnableDMAReq_UPDATE(FURI_HAL_SPEAKER_TIMER);
}

void wav_player_dma_stop() {
    LL_DMA_DisableChannel(DMA_INSTANCE);
}