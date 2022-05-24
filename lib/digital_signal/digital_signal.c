#include "digital_signal.h"

#include <furi.h>
#include <stm32wbxx_ll_dma.h>
#include <stm32wbxx_ll_tim.h>
#include <math.h>

#define F_TIM (64000000.0)
#define T_TIM (1.0 / F_TIM)

DigitalSignal* digital_signal_alloc(uint32_t max_edges_cnt) {
    DigitalSignal* signal = malloc(sizeof(DigitalSignal));
    signal->start_level = true;
    signal->edges_max_cnt = max_edges_cnt;
    signal->edge_timings = malloc(max_edges_cnt * sizeof(float));
    signal->reload_reg_buff = malloc(max_edges_cnt * sizeof(uint32_t));
    signal->edge_cnt = 0;

    return signal;
}

void digital_signal_free(DigitalSignal* signal) {
    furi_assert(signal);

    free(signal->edge_timings);
    free(signal->reload_reg_buff);
    free(signal);
}

bool digital_signal_append(DigitalSignal* signal_a, DigitalSignal* signal_b) {
    furi_assert(signal_a);
    furi_assert(signal_b);

    if(signal_a->edges_max_cnt < signal_a->edge_cnt + signal_b->edge_cnt) {
        return false;
    }

    bool end_level = signal_a->start_level;
    if(signal_a->edge_cnt) {
        end_level = signal_a->start_level ^ !(signal_a->edge_cnt % 2);
    }
    uint8_t start_copy = 0;
    if(end_level == signal_b->start_level) {
        if(signal_a->edge_cnt) {
            signal_a->edge_timings[signal_a->edge_cnt - 1] += signal_b->edge_timings[0];
            start_copy += 1;
        } else {
            signal_a->edge_timings[signal_a->edge_cnt] += signal_b->edge_timings[0];
        }
    }
    memcpy(
        &signal_a->edge_timings[signal_a->edge_cnt],
        &signal_b->edge_timings[start_copy],
        (signal_b->edge_cnt - start_copy) * sizeof(float));
    signal_a->edge_cnt += signal_b->edge_cnt - start_copy;

    return true;
}

bool digital_signal_get_start_level(DigitalSignal* signal) {
    furi_assert(signal);

    return signal->start_level;
}

uint32_t digital_signal_get_edges_cnt(DigitalSignal* signal) {
    furi_assert(signal);

    return signal->edge_cnt;
}

float digital_signal_get_edge(DigitalSignal* signal, uint32_t edge_num) {
    furi_assert(signal);
    furi_assert(edge_num < signal->edge_cnt);

    return signal->edge_timings[edge_num];
}

static void digital_signal_prepare_arr(DigitalSignal* signal) {
    float t_signal = 0;
    float t_current = 0;
    float r = 0;
    float r_int = 0;
    float r_dec = 0;

    for(size_t i = 0; i < signal->edge_cnt - 1; i++) {
        t_signal += signal->edge_timings[i];
        r = (t_signal - t_current) / T_TIM;
        r_dec = modff(r, &r_int);
        if(r_dec < 0.5f) {
            signal->reload_reg_buff[i] = (uint32_t)r_int - 1;
        } else {
            signal->reload_reg_buff[i] = (uint32_t)r_int;
        }
        t_current += (signal->reload_reg_buff[i] + 1) * T_TIM;
    }
}

bool digital_signal_send(DigitalSignal* signal, const GpioPin* gpio) {
    furi_assert(signal);
    furi_assert(gpio);

    // Configure gpio as output
    furi_hal_gpio_init(gpio, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    // Init gpio buffer and DMA channel
    uint16_t gpio_reg = gpio->port->ODR;
    uint16_t gpio_buff[2];
    if(signal->start_level) {
        gpio_buff[0] = gpio_reg | gpio->pin;
        gpio_buff[1] = gpio_reg & ~(gpio->pin);
    } else {
        gpio_buff[0] = gpio_reg & ~(gpio->pin);
        gpio_buff[1] = gpio_reg | gpio->pin;
    }
    LL_DMA_InitTypeDef dma_config = {};
    dma_config.MemoryOrM2MDstAddress = (uint32_t)gpio_buff;
    dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (gpio->port->ODR);
    dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    dma_config.Mode = LL_DMA_MODE_CIRCULAR;
    dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
    dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
    dma_config.NbData = 2;
    dma_config.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
    dma_config.Priority = LL_DMA_PRIORITY_VERYHIGH;
    LL_DMA_Init(DMA1, LL_DMA_CHANNEL_1, &dma_config);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, 2);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);

    // Init timer arr register buffer and DMA channel
    digital_signal_prepare_arr(signal);
    dma_config.MemoryOrM2MDstAddress = (uint32_t)signal->reload_reg_buff;
    dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (TIM2->ARR);
    dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    dma_config.Mode = LL_DMA_MODE_NORMAL;
    dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    dma_config.NbData = signal->edge_cnt - 2;
    dma_config.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
    dma_config.Priority = LL_DMA_PRIORITY_HIGH;
    LL_DMA_Init(DMA1, LL_DMA_CHANNEL_2, &dma_config);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, signal->edge_cnt - 2);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);

    // Set up timer
    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockDivision(TIM2, LL_TIM_CLOCKDIVISION_DIV1);
    LL_TIM_SetPrescaler(TIM2, 0);
    LL_TIM_SetAutoReload(TIM2, 10);
    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableUpdateEvent(TIM2);
    LL_TIM_EnableDMAReq_UPDATE(TIM2);

    // Start transactions
    LL_TIM_GenerateEvent_UPDATE(TIM2); // Do we really need it?
    LL_TIM_EnableCounter(TIM2);

    while(!LL_DMA_IsActiveFlag_TC2(DMA1))
        ;

    LL_DMA_ClearFlag_TC1(DMA1);
    LL_DMA_ClearFlag_TC2(DMA1);
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_SetCounter(TIM2, 0);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);

    return true;
}
