/**
  * @file infrared_app_signal.h
  * Infrared: Signal class
  */
#pragma once
#include <infrared_worker.h>
#include <stdint.h>
#include <string>
#include <infrared.h>

/** Infrared application signal class */
class InfraredAppSignal {
public:
    /** Raw signal structure */
    typedef struct {
        /** Timings amount */
        size_t timings_cnt;
        /** Samples of raw signal in ms */
        uint32_t* timings;
        /** PWM Frequency of raw signal */
        uint32_t frequency;
        /** PWM Duty cycle of raw signal */
        float duty_cycle;
    } RawSignal;

private:
    /** if true - signal is raw, if false - signal is parsed */
    bool raw_signal;
    /** signal data, either raw or parsed */
    union {
        /** signal data for parsed signal */
        InfraredMessage message;
        /** raw signal data */
        RawSignal raw;
    } payload;

    /** Copy raw signal into object
     *
     * @param timings - timings (samples) of raw signal
     * @param size - number of timings
     * @frequency - PWM frequency of raw signal
     * @duty_cycle - PWM duty cycle
     */
    void
        copy_raw_signal(const uint32_t* timings, size_t size, uint32_t frequency, float duty_cycle);
    /** Clear and free timings data */
    void clear_timings();

public:
    /** Construct Infrared signal class */
    InfraredAppSignal() {
        raw_signal = false;
        payload.message.protocol = InfraredProtocolUnknown;
    }

    /** Destruct signal class and free all allocated data */
    ~InfraredAppSignal() {
        clear_timings();
    }

    /** Construct object with raw signal
     *
     * @param timings - timings (samples) of raw signal
     * @param size - number of timings
     * @frequency - PWM frequency of raw signal
     * @duty_cycle - PWM duty cycle
     */
    InfraredAppSignal(
        const uint32_t* timings,
        size_t timings_cnt,
        uint32_t frequency,
        float duty_cycle);

    /** Construct object with parsed signal
     *
     * @param infrared_message - parsed_signal to construct from
     */
    InfraredAppSignal(const InfraredMessage* infrared_message);

    /** Copy constructor */
    InfraredAppSignal(const InfraredAppSignal& other);
    /** Move constructor */
    InfraredAppSignal(InfraredAppSignal&& other);

    /** Assignment operator */
    InfraredAppSignal& operator=(const InfraredAppSignal& signal);

    /** Set object to parsed signal
     *
     * @param infrared_message - parsed_signal to construct from
     */
    void set_message(const InfraredMessage* infrared_message);

    /** Set object to raw signal
     *
     * @param timings - timings (samples) of raw signal
     * @param size - number of timings
     * @frequency - PWM frequency of raw signal
     * @duty_cycle - PWM duty cycle
     */
    void
        set_raw_signal(uint32_t* timings, size_t timings_cnt, uint32_t frequency, float duty_cycle);

    /** Transmit held signal (???) */
    void transmit() const;

    /** Show is held signal raw
     *
     * @retval true if signal is raw, false if signal is parsed
     */
    bool is_raw(void) const {
        return raw_signal;
    }

    /** Get parsed signal.
     * User must check is_raw() signal before calling this function.
     *
     * @retval parsed signal pointer
     */
    const InfraredMessage& get_message(void) const {
        furi_assert(!raw_signal);
        return payload.message;
    }

    /** Get raw signal.
     * User must check is_raw() signal before calling this function.
     *
     * @retval raw signal
     */
    const RawSignal& get_raw_signal(void) const {
        furi_assert(raw_signal);
        return payload.raw;
    }
};
