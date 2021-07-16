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
    } RawSignal;

private:
    bool decoded;
    union {
        IrdaMessage message;
        RawSignal raw;
    } payload;

    void copy_timings(const uint32_t* timings, size_t size);
    void clear_timings();

public:
    IrdaAppSignal() {
        decoded = true;
        payload.message.protocol = IrdaProtocolUnknown;
    }

    ~IrdaAppSignal() {
        clear_timings();
    }

    IrdaAppSignal(const uint32_t* timings, size_t timings_cnt);
    IrdaAppSignal(const IrdaMessage* irda_message);

    IrdaAppSignal(const IrdaAppSignal& other);
    IrdaAppSignal(IrdaAppSignal&& other);

    IrdaAppSignal& operator=(const IrdaAppSignal& signal);

    void set_message(const IrdaMessage* irda_message);
    void set_raw_signal(uint32_t* timings, size_t timings_cnt);
    void copy_raw_signal(uint32_t* timings, size_t timings_cnt);

    void transmit() const;

    bool is_raw(void) const {
        return !decoded;
    }

    const IrdaMessage& get_message(void) const {
        furi_assert(decoded);
        return payload.message;
    }

    const RawSignal& get_raw_signal(void) const {
        furi_assert(!decoded);
        return payload.raw;
    }
};
