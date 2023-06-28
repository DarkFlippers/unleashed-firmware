#include "digital_signal.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_resources.h>
#include <math.h>

#include <stm32wbxx_ll_dma.h>
#include <stm32wbxx_ll_tim.h>

/* must be on bank B */
// For debugging purposes use `--extra-define=DIGITAL_SIGNAL_DEBUG_OUTPUT_PIN=gpio_ext_pb3` fbt option

struct ReloadBuffer {
    uint32_t* buffer; /* DMA ringbuffer */
    uint32_t size; /* maximum entry count of the ring buffer */
    uint32_t write_pos; /* current buffer write index */
    uint32_t read_pos; /* current buffer read index */
    bool dma_active;
};

struct DigitalSequence {
    uint8_t signals_size;
    bool bake;
    uint32_t sequence_used;
    uint32_t sequence_size;
    DigitalSignal** signals;
    uint8_t* sequence;
    const GpioPin* gpio;
    uint32_t send_time;
    bool send_time_active;
    LL_DMA_InitTypeDef dma_config_gpio;
    LL_DMA_InitTypeDef dma_config_timer;
    uint32_t* gpio_buff;
    struct ReloadBuffer* dma_buffer;
};

struct DigitalSignalInternals {
    uint64_t factor;
    uint32_t reload_reg_entries;
    uint32_t reload_reg_remainder;
    uint32_t gpio_buff[2];
    const GpioPin* gpio;
    LL_DMA_InitTypeDef dma_config_gpio;
    LL_DMA_InitTypeDef dma_config_timer;
};

#define TAG "DigitalSignal"

#define F_TIM (64000000.0)
#define T_TIM 1562 /* 15.625 ns *100 */
#define T_TIM_DIV2 781 /* 15.625 ns / 2 *100 */

/* end marker in DMA ringbuffer, will get written into timer register at the end */
#define SEQ_TIMER_MAX 0xFFFFFFFF

/* time to wait in loops before returning */
#define SEQ_LOCK_WAIT_MS 10UL
#define SEQ_LOCK_WAIT_TICKS (SEQ_LOCK_WAIT_MS * 1000 * 64)

/* maximum entry count of the sequence dma ring buffer */
#define RINGBUFFER_SIZE 128

/* maximum number of DigitalSignals in a sequence */
#define SEQUENCE_SIGNALS_SIZE 32
/*
 * if sequence size runs out from the initial value passed to digital_sequence_alloc
 * the size will be increased by this amount and reallocated
 */
#define SEQUENCE_SIZE_REALLOCATE_INCREMENT 256

DigitalSignal* digital_signal_alloc(uint32_t max_edges_cnt) {
    DigitalSignal* signal = malloc(sizeof(DigitalSignal));
    signal->start_level = true;
    signal->edges_max_cnt = max_edges_cnt;
    signal->edge_timings = malloc(signal->edges_max_cnt * sizeof(uint32_t));
    signal->edge_cnt = 0;
    signal->reload_reg_buff = malloc(signal->edges_max_cnt * sizeof(uint32_t));

    signal->internals = malloc(sizeof(DigitalSignalInternals));
    DigitalSignalInternals* internals = signal->internals;

    internals->factor = 1024 * 1024;

    internals->dma_config_gpio.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    internals->dma_config_gpio.Mode = LL_DMA_MODE_CIRCULAR;
    internals->dma_config_gpio.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    internals->dma_config_gpio.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    internals->dma_config_gpio.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    internals->dma_config_gpio.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    internals->dma_config_gpio.NbData = 2;
    internals->dma_config_gpio.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
    internals->dma_config_gpio.Priority = LL_DMA_PRIORITY_VERYHIGH;

    internals->dma_config_timer.PeriphOrM2MSrcAddress = (uint32_t) & (TIM2->ARR);
    internals->dma_config_timer.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    internals->dma_config_timer.Mode = LL_DMA_MODE_NORMAL;
    internals->dma_config_timer.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    internals->dma_config_timer.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    internals->dma_config_timer.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    internals->dma_config_timer.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    internals->dma_config_timer.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
    internals->dma_config_timer.Priority = LL_DMA_PRIORITY_HIGH;

    return signal;
}

void digital_signal_free(DigitalSignal* signal) {
    furi_assert(signal);

    free(signal->edge_timings);
    free(signal->reload_reg_buff);
    free(signal->internals);
    free(signal);
}

bool digital_signal_append(DigitalSignal* signal_a, DigitalSignal* signal_b) {
    furi_assert(signal_a);
    furi_assert(signal_b);

    if(signal_a->edges_max_cnt < signal_a->edge_cnt + signal_b->edge_cnt) {
        return false;
    }
    /* in case there are no edges in our target signal, the signal to append makes the rules */
    if(!signal_a->edge_cnt) {
        signal_a->start_level = signal_b->start_level;
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

    for(size_t i = 0; i < signal_b->edge_cnt - start_copy; i++) {
        signal_a->edge_timings[signal_a->edge_cnt + i] = signal_b->edge_timings[start_copy + i];
    }
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

void digital_signal_add(DigitalSignal* signal, uint32_t ticks) {
    furi_assert(signal);
    furi_assert(signal->edge_cnt < signal->edges_max_cnt);

    signal->edge_timings[signal->edge_cnt++] = ticks;
}

void digital_signal_add_pulse(DigitalSignal* signal, uint32_t ticks, bool level) {
    furi_assert(signal);
    furi_assert(signal->edge_cnt < signal->edges_max_cnt);

    /* virgin signal? add it as the only level */
    if(signal->edge_cnt == 0) {
        signal->start_level = level;
        signal->edge_timings[signal->edge_cnt++] = ticks;
    } else {
        bool end_level = signal->start_level ^ !(signal->edge_cnt % 2);

        if(level != end_level) {
            signal->edge_timings[signal->edge_cnt++] = ticks;
        } else {
            signal->edge_timings[signal->edge_cnt - 1] += ticks;
        }
    }
}

uint32_t digital_signal_get_edge(DigitalSignal* signal, uint32_t edge_num) {
    furi_assert(signal);
    furi_assert(edge_num < signal->edge_cnt);

    return signal->edge_timings[edge_num];
}

void digital_signal_prepare_arr(DigitalSignal* signal) {
    furi_assert(signal);

    DigitalSignalInternals* internals = signal->internals;

    /* set up signal polarities */
    if(internals->gpio) {
        uint32_t bit_set = internals->gpio->pin;
        uint32_t bit_reset = internals->gpio->pin << 16;

#ifdef DIGITAL_SIGNAL_DEBUG_OUTPUT_PIN
        bit_set |= DIGITAL_SIGNAL_DEBUG_OUTPUT_PIN.pin;
        bit_reset |= DIGITAL_SIGNAL_DEBUG_OUTPUT_PIN.pin << 16;
#endif

        if(signal->start_level) {
            internals->gpio_buff[0] = bit_set;
            internals->gpio_buff[1] = bit_reset;
        } else {
            internals->gpio_buff[0] = bit_reset;
            internals->gpio_buff[1] = bit_set;
        }
    }

    /* set up edge timings */
    internals->reload_reg_entries = 0;

    for(size_t pos = 0; pos < signal->edge_cnt; pos++) {
        uint32_t pulse_duration = signal->edge_timings[pos] + internals->reload_reg_remainder;
        if(pulse_duration < 10 || pulse_duration > 10000000) {
            FURI_LOG_D(
                TAG,
                "[prepare] pulse_duration out of range: %lu = %lu * %llu",
                pulse_duration,
                signal->edge_timings[pos],
                internals->factor);
            pulse_duration = 100;
        }
        uint32_t pulse_ticks = (pulse_duration + T_TIM_DIV2) / T_TIM;
        internals->reload_reg_remainder = pulse_duration - (pulse_ticks * T_TIM);

        if(pulse_ticks > 1) {
            signal->reload_reg_buff[internals->reload_reg_entries++] = pulse_ticks - 1;
        }
    }
}

static void digital_signal_stop_dma() {
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_ClearFlag_TC1(DMA1);
    LL_DMA_ClearFlag_TC2(DMA1);
}

static void digital_signal_stop_timer() {
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_DisableUpdateEvent(TIM2);
    LL_TIM_DisableDMAReq_UPDATE(TIM2);

    furi_hal_bus_disable(FuriHalBusTIM2);
}

static void digital_signal_setup_timer() {
    furi_hal_bus_enable(FuriHalBusTIM2);

    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockDivision(TIM2, LL_TIM_CLOCKDIVISION_DIV1);
    LL_TIM_SetPrescaler(TIM2, 0);
    LL_TIM_SetAutoReload(TIM2, SEQ_TIMER_MAX);
    LL_TIM_SetCounter(TIM2, 0);
}

static void digital_signal_start_timer() {
    LL_TIM_EnableCounter(TIM2);
    LL_TIM_EnableUpdateEvent(TIM2);
    LL_TIM_EnableDMAReq_UPDATE(TIM2);
    LL_TIM_GenerateEvent_UPDATE(TIM2);
}

static bool digital_signal_setup_dma(DigitalSignal* signal) {
    furi_assert(signal);
    DigitalSignalInternals* internals = signal->internals;

    if(!signal->internals->reload_reg_entries) {
        return false;
    }
    digital_signal_stop_dma();

    internals->dma_config_gpio.MemoryOrM2MDstAddress = (uint32_t)internals->gpio_buff;
    internals->dma_config_gpio.PeriphOrM2MSrcAddress = (uint32_t) & (internals->gpio->port->BSRR);
    internals->dma_config_timer.MemoryOrM2MDstAddress = (uint32_t)signal->reload_reg_buff;
    internals->dma_config_timer.NbData = signal->internals->reload_reg_entries;

    /* set up DMA channel 1 and 2 for GPIO and timer copy operations */
    LL_DMA_Init(DMA1, LL_DMA_CHANNEL_1, &internals->dma_config_gpio);
    LL_DMA_Init(DMA1, LL_DMA_CHANNEL_2, &internals->dma_config_timer);

    /* enable both DMA channels */
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);

    return true;
}

void digital_signal_send(DigitalSignal* signal, const GpioPin* gpio) {
    furi_assert(signal);

    if(!signal->edge_cnt) {
        return;
    }

    /* Configure gpio as output */
    signal->internals->gpio = gpio;
    furi_hal_gpio_init(
        signal->internals->gpio, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    digital_signal_prepare_arr(signal);

    digital_signal_setup_dma(signal);
    digital_signal_setup_timer();
    digital_signal_start_timer();

    while(!LL_DMA_IsActiveFlag_TC2(DMA1)) {
    }

    digital_signal_stop_timer();
    digital_signal_stop_dma();
}

static void digital_sequence_alloc_signals(DigitalSequence* sequence, uint32_t size) {
    sequence->signals_size = size;
    sequence->signals = malloc(sequence->signals_size * sizeof(DigitalSignal*));
}

static void digital_sequence_alloc_sequence(DigitalSequence* sequence, uint32_t size) {
    sequence->sequence_used = 0;
    sequence->sequence_size = size;
    sequence->sequence = malloc(sequence->sequence_size);
    sequence->send_time = 0;
    sequence->send_time_active = false;
}

DigitalSequence* digital_sequence_alloc(uint32_t size, const GpioPin* gpio) {
    furi_assert(gpio);

    DigitalSequence* sequence = malloc(sizeof(DigitalSequence));

    sequence->gpio = gpio;
    sequence->bake = false;

    sequence->dma_buffer = malloc(sizeof(struct ReloadBuffer));
    sequence->dma_buffer->size = RINGBUFFER_SIZE;
    sequence->dma_buffer->buffer = malloc(sequence->dma_buffer->size * sizeof(uint32_t));

    sequence->dma_config_gpio.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    sequence->dma_config_gpio.Mode = LL_DMA_MODE_CIRCULAR;
    sequence->dma_config_gpio.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    sequence->dma_config_gpio.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    sequence->dma_config_gpio.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    sequence->dma_config_gpio.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    sequence->dma_config_gpio.NbData = 2;
    sequence->dma_config_gpio.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
    sequence->dma_config_gpio.Priority = LL_DMA_PRIORITY_VERYHIGH;

    sequence->dma_config_timer.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    sequence->dma_config_timer.Mode = LL_DMA_MODE_CIRCULAR;
    sequence->dma_config_timer.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    sequence->dma_config_timer.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    sequence->dma_config_timer.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    sequence->dma_config_timer.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    sequence->dma_config_timer.PeriphOrM2MSrcAddress = (uint32_t) & (TIM2->ARR);
    sequence->dma_config_timer.MemoryOrM2MDstAddress = (uint32_t)sequence->dma_buffer->buffer;
    sequence->dma_config_timer.NbData = sequence->dma_buffer->size;
    sequence->dma_config_timer.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
    sequence->dma_config_timer.Priority = LL_DMA_PRIORITY_HIGH;

    digital_sequence_alloc_signals(sequence, SEQUENCE_SIGNALS_SIZE);
    digital_sequence_alloc_sequence(sequence, size);

    return sequence;
}

void digital_sequence_free(DigitalSequence* sequence) {
    furi_assert(sequence);

    free(sequence->signals);
    free(sequence->sequence);
    free(sequence->dma_buffer->buffer);
    free(sequence->dma_buffer);
    free(sequence);
}

void digital_sequence_set_signal(
    DigitalSequence* sequence,
    uint8_t signal_index,
    DigitalSignal* signal) {
    furi_assert(sequence);
    furi_assert(signal);
    furi_assert(signal_index < sequence->signals_size);

    sequence->signals[signal_index] = signal;
    signal->internals->gpio = sequence->gpio;
    signal->internals->reload_reg_remainder = 0;

    digital_signal_prepare_arr(signal);
}

void digital_sequence_set_sendtime(DigitalSequence* sequence, uint32_t send_time) {
    furi_assert(sequence);

    sequence->send_time = send_time;
    sequence->send_time_active = true;
}

void digital_sequence_add(DigitalSequence* sequence, uint8_t signal_index) {
    furi_assert(sequence);
    furi_assert(signal_index < sequence->signals_size);

    if(sequence->sequence_used >= sequence->sequence_size) {
        sequence->sequence_size += SEQUENCE_SIZE_REALLOCATE_INCREMENT;
        sequence->sequence = realloc(sequence->sequence, sequence->sequence_size); //-V701
        furi_assert(sequence->sequence);
    }

    sequence->sequence[sequence->sequence_used++] = signal_index;
}

static bool digital_sequence_setup_dma(DigitalSequence* sequence) {
    furi_assert(sequence);

    digital_signal_stop_dma();

    sequence->dma_config_gpio.MemoryOrM2MDstAddress = (uint32_t)sequence->gpio_buff;
    sequence->dma_config_gpio.PeriphOrM2MSrcAddress = (uint32_t) & (sequence->gpio->port->BSRR);

    /* set up DMA channel 1 and 2 for GPIO and timer copy operations */
    LL_DMA_Init(DMA1, LL_DMA_CHANNEL_1, &sequence->dma_config_gpio);
    LL_DMA_Init(DMA1, LL_DMA_CHANNEL_2, &sequence->dma_config_timer);

    /* enable both DMA channels */
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);

    return true;
}

static DigitalSignal* digital_sequence_bake(DigitalSequence* sequence) {
    furi_assert(sequence);

    uint32_t edges = 0;

    for(uint32_t pos = 0; pos < sequence->sequence_used; pos++) {
        uint8_t signal_index = sequence->sequence[pos];
        DigitalSignal* sig = sequence->signals[signal_index];

        edges += sig->edge_cnt;
    }

    DigitalSignal* ret = digital_signal_alloc(edges);

    for(uint32_t pos = 0; pos < sequence->sequence_used; pos++) {
        uint8_t signal_index = sequence->sequence[pos];
        DigitalSignal* sig = sequence->signals[signal_index];

        digital_signal_append(ret, sig);
    }

    return ret;
}

static void digital_sequence_finish(DigitalSequence* sequence) {
    struct ReloadBuffer* dma_buffer = sequence->dma_buffer;

    if(dma_buffer->dma_active) {
        uint32_t prev_timer = DWT->CYCCNT;
        do {
            /* we are finished, when the DMA transferred the SEQ_TIMER_MAX marker */
            if(TIM2->ARR == SEQ_TIMER_MAX) {
                break;
            }
            if(DWT->CYCCNT - prev_timer > SEQ_LOCK_WAIT_TICKS) {
                dma_buffer->read_pos =
                    RINGBUFFER_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_2);
                FURI_LOG_D(
                    TAG,
                    "[SEQ] hung %lu ms in finish (ARR 0x%08lx, read %lu, write %lu)",
                    SEQ_LOCK_WAIT_MS,
                    TIM2->ARR,
                    dma_buffer->read_pos,
                    dma_buffer->write_pos);
                break;
            }
        } while(1);
    }

    digital_signal_stop_timer();
    digital_signal_stop_dma();
}

static void digital_sequence_queue_pulse(DigitalSequence* sequence, uint32_t length) {
    struct ReloadBuffer* dma_buffer = sequence->dma_buffer;

    if(dma_buffer->dma_active) {
        uint32_t prev_timer = DWT->CYCCNT;
        do {
            dma_buffer->read_pos = RINGBUFFER_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_2);

            uint32_t free =
                (RINGBUFFER_SIZE + dma_buffer->read_pos - dma_buffer->write_pos) % RINGBUFFER_SIZE;

            if(free > 2) {
                break;
            }

            if(DWT->CYCCNT - prev_timer > SEQ_LOCK_WAIT_TICKS) {
                FURI_LOG_D(
                    TAG,
                    "[SEQ] hung %lu ms in queue (ARR 0x%08lx, read %lu, write %lu)",
                    SEQ_LOCK_WAIT_MS,
                    TIM2->ARR,
                    dma_buffer->read_pos,
                    dma_buffer->write_pos);
                break;
            }
            if(TIM2->ARR == SEQ_TIMER_MAX) {
                FURI_LOG_D(
                    TAG,
                    "[SEQ] buffer underrun in queue (ARR 0x%08lx, read %lu, write %lu)",
                    TIM2->ARR,
                    dma_buffer->read_pos,
                    dma_buffer->write_pos);
                break;
            }
        } while(1);
    }

    dma_buffer->buffer[dma_buffer->write_pos] = length;
    dma_buffer->write_pos++;
    dma_buffer->write_pos %= RINGBUFFER_SIZE;
    dma_buffer->buffer[dma_buffer->write_pos] = SEQ_TIMER_MAX;
}

bool digital_sequence_send(DigitalSequence* sequence) {
    furi_assert(sequence);

    struct ReloadBuffer* dma_buffer = sequence->dma_buffer;

    furi_hal_gpio_init(sequence->gpio, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
#ifdef DIGITAL_SIGNAL_DEBUG_OUTPUT_PIN
    furi_hal_gpio_init(
        &DIGITAL_SIGNAL_DEBUG_OUTPUT_PIN, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
#endif

    if(sequence->bake) {
        DigitalSignal* sig = digital_sequence_bake(sequence);

        digital_signal_send(sig, sequence->gpio);
        digital_signal_free(sig);
        return true;
    }

    if(!sequence->sequence_used) {
        return false;
    }

    int32_t remainder = 0;
    uint32_t trade_for_next = 0;
    uint32_t seq_pos_next = 1;

    dma_buffer->dma_active = false;
    dma_buffer->buffer[0] = SEQ_TIMER_MAX;
    dma_buffer->read_pos = 0;
    dma_buffer->write_pos = 0;

    /* already prepare the current signal pointer */
    DigitalSignal* sig = sequence->signals[sequence->sequence[0]];
    DigitalSignal* sig_next = NULL;
    /* re-use the GPIO buffer from the first signal */
    sequence->gpio_buff = sig->internals->gpio_buff;

    FURI_CRITICAL_ENTER();

    while(sig) {
        bool last_signal = (seq_pos_next >= sequence->sequence_used);

        if(!last_signal) {
            sig_next = sequence->signals[sequence->sequence[seq_pos_next++]];
        }

        for(uint32_t pulse_pos = 0; pulse_pos < sig->internals->reload_reg_entries; pulse_pos++) {
            bool last_pulse = ((pulse_pos + 1) >= sig->internals->reload_reg_entries);
            uint32_t pulse_length = sig->reload_reg_buff[pulse_pos] + trade_for_next;

            trade_for_next = 0;

            /* when we are too late more than half a tick, make the first edge temporarily longer */
            if(remainder >= T_TIM_DIV2) {
                remainder -= T_TIM;
                pulse_length += 1;
            }

            /* last pulse in current signal and have a next signal? */
            if(last_pulse && sig_next) {
                /* when a signal ends with the same level as the next signal begins, let the next signal generate the whole pulse.
                   beware, we do not want the level after the last edge, but the last level before that edge */
                bool end_level = sig->start_level ^ ((sig->edge_cnt % 2) == 0);

                /* if they have the same level, pass the duration to the next pulse(s) */
                if(end_level == sig_next->start_level) {
                    trade_for_next = pulse_length;
                }
            }

            /* if it was decided, that the next signal's first pulse shall also handle our "length", then do not queue here */
            if(!trade_for_next) {
                digital_sequence_queue_pulse(sequence, pulse_length);

                if(!dma_buffer->dma_active) {
                    /* start transmission when buffer was filled enough */
                    bool start_send = sequence->dma_buffer->write_pos >= (RINGBUFFER_SIZE - 2);

                    /* or it was the last pulse */
                    if(last_pulse && last_signal) {
                        start_send = true;
                    }

                    /* start transmission */
                    if(start_send) {
                        digital_sequence_setup_dma(sequence);
                        digital_signal_setup_timer();

                        /* if the send time is specified, wait till the core timer passed beyond that time */
                        if(sequence->send_time_active) {
                            sequence->send_time_active = false;
                            while(sequence->send_time - DWT->CYCCNT < 0x80000000) {
                            }
                        }
                        digital_signal_start_timer();
                        dma_buffer->dma_active = true;
                    }
                }
            }
        }

        remainder += sig->internals->reload_reg_remainder;
        sig = sig_next;
        sig_next = NULL;
    }

    /* wait until last dma transaction was finished */
    FURI_CRITICAL_EXIT();
    digital_sequence_finish(sequence);

    return true;
}

void digital_sequence_clear(DigitalSequence* sequence) {
    furi_assert(sequence);

    sequence->sequence_used = 0;
}

void digital_sequence_timebase_correction(DigitalSequence* sequence, float factor) {
    for(uint32_t sig_pos = 0; sig_pos < sequence->signals_size; sig_pos++) {
        DigitalSignal* signal = sequence->signals[sig_pos];

        if(signal) {
            signal->internals->factor = (uint32_t)(1024 * 1024 * factor);
            digital_signal_prepare_arr(signal);
        }
    }
}
