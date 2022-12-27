#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <furi_hal_gpio.h>
#include <stm32wbxx_ll_dma.h>
#include <stm32wbxx_ll_tim.h>

#ifdef __cplusplus
extern "C" {
#endif

/* helper for easier signal generation */
#define DIGITAL_SIGNAL_MS(x) (x * 100000000UL)
#define DIGITAL_SIGNAL_US(x) (x * 100000UL)
#define DIGITAL_SIGNAL_NS(x) (x * 100UL)
#define DIGITAL_SIGNAL_PS(x) (x / 10UL)

typedef struct {
    bool start_level;
    uint32_t edge_cnt;
    uint32_t edges_max_cnt;
    uint32_t* edge_timings;
    uint32_t* reload_reg_buff;
    uint32_t reload_reg_entries;
    uint32_t reload_reg_remainder;
    uint32_t gpio_buff[2];
    const GpioPin* gpio;
    LL_DMA_InitTypeDef dma_config_gpio;
    LL_DMA_InitTypeDef dma_config_timer;
} DigitalSignal;

typedef struct {
    uint8_t signals_size;
    bool bake;
    uint32_t sequence_used;
    uint32_t sequence_size;
    DigitalSignal** signals;
    bool* signals_prolonged;
    uint8_t* sequence;
    const GpioPin* gpio;
    uint32_t send_time;
} DigitalSequence;

DigitalSignal* digital_signal_alloc(uint32_t max_edges_cnt);

void digital_signal_free(DigitalSignal* signal);

void digital_signal_add(DigitalSignal* signal, uint32_t ticks);

void digital_signal_add_pulse(DigitalSignal* signal, uint32_t ticks, bool level);

bool digital_signal_append(DigitalSignal* signal_a, DigitalSignal* signal_b);

void digital_signal_prepare(DigitalSignal* signal);

bool digital_signal_get_start_level(DigitalSignal* signal);

uint32_t digital_signal_get_edges_cnt(DigitalSignal* signal);

uint32_t digital_signal_get_edge(DigitalSignal* signal, uint32_t edge_num);

void digital_signal_send(DigitalSignal* signal, const GpioPin* gpio);

DigitalSequence* digital_sequence_alloc(uint32_t size, const GpioPin* gpio);
void digital_sequence_free(DigitalSequence* sequence);
void digital_sequence_set_signal(
    DigitalSequence* sequence,
    uint8_t signal_index,
    DigitalSignal* signal);
void digital_sequence_set_sendtime(DigitalSequence* sequence, uint32_t send_time);
void digital_sequence_add(DigitalSequence* sequence, uint8_t signal_index);
bool digital_sequence_send(DigitalSequence* sequence);
void digital_sequence_clear(DigitalSequence* sequence);

#ifdef __cplusplus
}
#endif
