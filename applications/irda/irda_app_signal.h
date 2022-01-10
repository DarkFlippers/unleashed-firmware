#pragma once
#include <irda_worker.h>
#include <stdint.h>
#include <string>
#include <irda.h>

class IrdaAppSignal {
public:
    typedef struct {
        size_t timings_cnt;
        uint32_t* timings;
        uint32_t frequency;
        float duty_cycle;
    } RawSignal;

private:
    bool raw_signal;
    union {
        IrdaMessage message;
        RawSignal raw;
    } payload;

    void
        copy_raw_signal(const uint32_t* timings, size_t size, uint32_t frequency, float duty_cycle);
    void clear_timings();

public:
    IrdaAppSignal() {
        raw_signal = false;
        payload.message.protocol = IrdaProtocolUnknown;
    }

    ~IrdaAppSignal() {
        clear_timings();
    }

    IrdaAppSignal(
        const uint32_t* timings,
        size_t timings_cnt,
        uint32_t frequency,
        float duty_cycle);
    IrdaAppSignal(const IrdaMessage* irda_message);

    IrdaAppSignal(const IrdaAppSignal& other);
    IrdaAppSignal(IrdaAppSignal&& other);

    IrdaAppSignal& operator=(const IrdaAppSignal& signal);

    void set_message(const IrdaMessage* irda_message);
    void
        set_raw_signal(uint32_t* timings, size_t timings_cnt, uint32_t frequency, float duty_cycle);

    void transmit() const;

    bool is_raw(void) const {
        return raw_signal;
    }

    const IrdaMessage& get_message(void) const {
        furi_assert(!raw_signal);
        return payload.message;
    }

    const RawSignal& get_raw_signal(void) const {
        furi_assert(raw_signal);
        return payload.raw;
    }
};
