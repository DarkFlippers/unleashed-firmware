#pragma once
#include "stdint.h"

class PulseJoiner {
public:
    /**
     * @brief Push timer pulse. First negative pulse is ommited.
     * 
     * @param polarity pulse polarity: true = high2low, false = low2high
     * @param period overall period time in timer clicks
     * @param pulse pulse time in timer clicks
     * 
     * @return true - next pulse can and must be popped immediatly
     */
    bool push_pulse(bool polarity, uint16_t period, uint16_t pulse);

    /**
     * @brief Get the next timer pulse. Call only if push_pulse returns true.
     * 
     * @param period overall period time in timer clicks
     * @param pulse pulse time in timer clicks
     */
    void pop_pulse(uint16_t* period, uint16_t* pulse);

    PulseJoiner();

private:
    struct Pulse {
        bool polarity;
        uint16_t time;
    };

    uint8_t pulse_index = 0;
    static const uint8_t pulse_max = 6;
    Pulse pulses[pulse_max];
};
