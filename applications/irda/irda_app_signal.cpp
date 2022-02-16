#include "irda_app_signal.h"
#include <irda_transmit.h>

void IrdaAppSignal::copy_raw_signal(
    const uint32_t* timings,
    size_t size,
    uint32_t frequency,
    float duty_cycle) {
    furi_assert(size);
    furi_assert(timings);

    payload.raw.frequency = frequency;
    payload.raw.duty_cycle = duty_cycle;
    payload.raw.timings_cnt = size;
    if(size) {
        payload.raw.timings = new uint32_t[size];
        memcpy(payload.raw.timings, timings, size * sizeof(uint32_t));
    }
}

void IrdaAppSignal::clear_timings() {
    if(raw_signal) {
        delete[] payload.raw.timings;
        payload.raw.timings_cnt = 0;
        payload.raw.timings = nullptr;
    }
}

IrdaAppSignal::IrdaAppSignal(
    const uint32_t* timings,
    size_t timings_cnt,
    uint32_t frequency,
    float duty_cycle) {
    raw_signal = true;
    copy_raw_signal(timings, timings_cnt, frequency, duty_cycle);
}

IrdaAppSignal::IrdaAppSignal(const IrdaMessage* irda_message) {
    raw_signal = false;
    payload.message = *irda_message;
}

IrdaAppSignal& IrdaAppSignal::operator=(const IrdaAppSignal& other) {
    clear_timings();
    raw_signal = other.raw_signal;
    if(!raw_signal) {
        payload.message = other.payload.message;
    } else {
        copy_raw_signal(
            other.payload.raw.timings,
            other.payload.raw.timings_cnt,
            other.payload.raw.frequency,
            other.payload.raw.duty_cycle);
    }

    return *this;
}

IrdaAppSignal::IrdaAppSignal(const IrdaAppSignal& other) {
    raw_signal = other.raw_signal;
    if(!raw_signal) {
        payload.message = other.payload.message;
    } else {
        copy_raw_signal(
            other.payload.raw.timings,
            other.payload.raw.timings_cnt,
            other.payload.raw.frequency,
            other.payload.raw.duty_cycle);
    }
}

IrdaAppSignal::IrdaAppSignal(IrdaAppSignal&& other) {
    raw_signal = other.raw_signal;
    if(!raw_signal) {
        payload.message = other.payload.message;
    } else {
        furi_assert(other.payload.raw.timings_cnt > 0);

        payload.raw.timings = other.payload.raw.timings;
        payload.raw.timings_cnt = other.payload.raw.timings_cnt;
        payload.raw.frequency = other.payload.raw.frequency;
        payload.raw.duty_cycle = other.payload.raw.duty_cycle;
        other.payload.raw.timings = nullptr;
        other.payload.raw.timings_cnt = 0;
        other.raw_signal = false;
    }
}

void IrdaAppSignal::set_message(const IrdaMessage* irda_message) {
    clear_timings();
    raw_signal = false;
    payload.message = *irda_message;
}

void IrdaAppSignal::set_raw_signal(
    uint32_t* timings,
    size_t timings_cnt,
    uint32_t frequency,
    float duty_cycle) {
    clear_timings();
    raw_signal = true;
    copy_raw_signal(timings, timings_cnt, frequency, duty_cycle);
}

void IrdaAppSignal::transmit() const {
    if(!raw_signal) {
        irda_send(&payload.message, 1);
    } else {
        irda_send_raw_ext(
            payload.raw.timings,
            payload.raw.timings_cnt,
            true,
            payload.raw.frequency,
            payload.raw.duty_cycle);
    }
}
