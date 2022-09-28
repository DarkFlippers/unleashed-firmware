#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <infrared.h>
#include <flipper_format/flipper_format.h>

typedef struct InfraredSignal InfraredSignal;

typedef struct {
    size_t timings_size;
    uint32_t* timings;
    uint32_t frequency;
    float duty_cycle;
} InfraredRawSignal;

InfraredSignal* infrared_signal_alloc();
void infrared_signal_free(InfraredSignal* signal);

bool infrared_signal_is_raw(InfraredSignal* signal);
bool infrared_signal_is_valid(InfraredSignal* signal);

void infrared_signal_set_signal(InfraredSignal* signal, const InfraredSignal* other);

void infrared_signal_set_raw_signal(
    InfraredSignal* signal,
    const uint32_t* timings,
    size_t timings_size,
    uint32_t frequency,
    float duty_cycle);
InfraredRawSignal* infrared_signal_get_raw_signal(InfraredSignal* signal);

void infrared_signal_set_message(InfraredSignal* signal, const InfraredMessage* message);
InfraredMessage* infrared_signal_get_message(InfraredSignal* signal);

bool infrared_signal_save(InfraredSignal* signal, FlipperFormat* ff, const char* name);
bool infrared_signal_read(InfraredSignal* signal, FlipperFormat* ff, string_t name);
bool infrared_signal_search_and_read(
    InfraredSignal* signal,
    FlipperFormat* ff,
    const string_t name);

void infrared_signal_transmit(InfraredSignal* signal);
