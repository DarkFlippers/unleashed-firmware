#include "infrared_app_signal.h"
#include <infrared_transmit.h>

void InfraredAppSignal::copy_raw_signal(
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

void InfraredAppSignal::clear_timings() {
    if(raw_signal) {
        delete[] payload.raw.timings;
        payload.raw.timings_cnt = 0;
        payload.raw.timings = nullptr;
    }
}

InfraredAppSignal::InfraredAppSignal(
    const uint32_t* timings,
    size_t timings_cnt,
    uint32_t frequency,
    float duty_cycle) {
    raw_signal = true;
    copy_raw_signal(timings, timings_cnt, frequency, duty_cycle);
}

InfraredAppSignal::InfraredAppSignal(const InfraredMessage* infrared_message) {
    raw_signal = false;
    payload.message = *infrared_message;
}

InfraredAppSignal& InfraredAppSignal::operator=(const InfraredAppSignal& other) {
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

InfraredAppSignal::InfraredAppSignal(const InfraredAppSignal& other) {
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

InfraredAppSignal::InfraredAppSignal(InfraredAppSignal&& other) {
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

void InfraredAppSignal::set_message(const InfraredMessage* infrared_message) {
    clear_timings();
    raw_signal = false;
    payload.message = *infrared_message;
}

void InfraredAppSignal::set_raw_signal(
    uint32_t* timings,
    size_t timings_cnt,
    uint32_t frequency,
    float duty_cycle) {
    clear_timings();
    raw_signal = true;
    copy_raw_signal(timings, timings_cnt, frequency, duty_cycle);
}

void InfraredAppSignal::transmit() const {
    if(!raw_signal) {
        infrared_send(&payload.message, 1);
    } else {
        infrared_send_raw_ext(
            payload.raw.timings,
            payload.raw.timings_cnt,
            true,
            payload.raw.frequency,
            payload.raw.duty_cycle);
    }
}
