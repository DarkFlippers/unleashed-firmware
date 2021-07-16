#include "irda-app-signal.h"
#include <irda_transmit.h>

void IrdaAppSignal::copy_timings(const uint32_t* timings, size_t size) {
    furi_assert(size);
    furi_assert(timings);

    if(size) {
        payload.raw.timings = new uint32_t[size];
        payload.raw.timings_cnt = size;
        memcpy(payload.raw.timings, timings, size * sizeof(uint32_t));
    }
}

void IrdaAppSignal::clear_timings() {
    if(!decoded) {
        delete[] payload.raw.timings;
        payload.raw.timings_cnt = 0;
        payload.raw.timings = nullptr;
    }
}

IrdaAppSignal::IrdaAppSignal(const uint32_t* timings, size_t timings_cnt) {
    decoded = false;
    copy_timings(timings, timings_cnt);
}

IrdaAppSignal::IrdaAppSignal(const IrdaMessage* irda_message) {
    decoded = true;
    payload.message = *irda_message;
}

IrdaAppSignal& IrdaAppSignal::operator=(const IrdaAppSignal& other) {
    clear_timings();
    decoded = other.decoded;
    if(decoded) {
        payload.message = other.payload.message;
    } else {
        copy_timings(other.payload.raw.timings, other.payload.raw.timings_cnt);
    }

    return *this;
}

IrdaAppSignal::IrdaAppSignal(const IrdaAppSignal& other) {
    decoded = other.decoded;
    if(decoded) {
        payload.message = other.payload.message;
    } else {
        copy_timings(other.payload.raw.timings, other.payload.raw.timings_cnt);
    }
}

IrdaAppSignal::IrdaAppSignal(IrdaAppSignal&& other) {
    clear_timings();

    decoded = other.decoded;
    if(decoded) {
        payload.message = other.payload.message;
    } else {
        furi_assert(other.payload.raw.timings_cnt > 0);

        payload.raw.timings = other.payload.raw.timings;
        payload.raw.timings_cnt = other.payload.raw.timings_cnt;
        other.payload.raw.timings = nullptr;
        other.payload.raw.timings_cnt = 0;
    }
}

void IrdaAppSignal::set_message(const IrdaMessage* irda_message) {
    clear_timings();
    decoded = true;
    payload.message = *irda_message;
}

void IrdaAppSignal::set_raw_signal(uint32_t* timings, size_t timings_cnt) {
    clear_timings();
    decoded = false;
    payload.raw.timings = timings;
    payload.raw.timings_cnt = timings_cnt;
}

void IrdaAppSignal::copy_raw_signal(uint32_t* timings, size_t timings_cnt) {
    clear_timings();
    decoded = false;
    copy_timings(timings, timings_cnt);
}

void IrdaAppSignal::transmit() const {
    if(decoded) {
        irda_send(&payload.message, 1);
    } else {
        irda_send_raw(payload.raw.timings, payload.raw.timings_cnt, true);
    }
}
