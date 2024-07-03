#include "digital_signal.h"
#include "digital_signal_i.h"

#include <furi.h>

#define TAG "DigitalSignal"

DigitalSignal* digital_signal_alloc(uint32_t max_size) {
    DigitalSignal* signal = malloc(sizeof(DigitalSignal) + (max_size * sizeof(uint32_t)));

    signal->max_size = max_size;

    return signal;
}

void digital_signal_free(DigitalSignal* signal) {
    furi_check(signal);

    free(signal);
}

bool digital_signal_get_start_level(const DigitalSignal* signal) {
    furi_check(signal);

    return signal->start_level;
}

void digital_signal_set_start_level(DigitalSignal* signal, bool level) {
    furi_check(signal);

    signal->start_level = level;
}

uint32_t digital_signal_get_size(const DigitalSignal* signal) {
    furi_check(signal);

    return signal->size;
}

void digital_signal_add_period(DigitalSignal* signal, uint32_t ticks) {
    furi_check(signal);
    furi_check(signal->size < signal->max_size);

    const uint32_t duration = ticks + signal->remainder;

    uint32_t reload_value = duration / DIGITAL_SIGNAL_T_TIM;
    int32_t remainder = duration - reload_value * DIGITAL_SIGNAL_T_TIM;

    if(remainder >= DIGITAL_SIGNAL_T_TIM_DIV2) {
        reload_value += 1;
        remainder -= DIGITAL_SIGNAL_T_TIM;
    }

    furi_check(reload_value > 1);

    signal->data[signal->size++] = reload_value - 1;
    signal->remainder = remainder;
}

static void digital_signal_extend_last_period(DigitalSignal* signal, uint32_t ticks) {
    furi_assert(signal->size <= signal->max_size);

    const uint32_t reload_value_old = signal->data[signal->size - 1] + 1;
    const uint32_t duration = ticks + signal->remainder + reload_value_old * DIGITAL_SIGNAL_T_TIM;

    uint32_t reload_value = duration / DIGITAL_SIGNAL_T_TIM;
    int32_t remainder = duration - reload_value * DIGITAL_SIGNAL_T_TIM;

    if(remainder >= DIGITAL_SIGNAL_T_TIM_DIV2) {
        reload_value += 1;
        remainder -= DIGITAL_SIGNAL_T_TIM;
    }

    furi_check(reload_value > 1);

    signal->data[signal->size - 1] = reload_value - 1;
    signal->remainder = remainder;
}

void digital_signal_add_period_with_level(DigitalSignal* signal, uint32_t ticks, bool level) {
    furi_check(signal);

    if(signal->size == 0) {
        signal->start_level = level;
        digital_signal_add_period(signal, ticks);
    } else {
        const bool end_level = signal->start_level ^ !(signal->size % 2);

        if(level != end_level) {
            digital_signal_add_period(signal, ticks);
        } else {
            digital_signal_extend_last_period(signal, ticks);
        }
    }
}
