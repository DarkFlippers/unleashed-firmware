#pragma once
#include <stdint.h>

/**
 * This code tries to fit the periods into a given number of cycles (phases) by taking cycles from the next cycle of periods.
 */
class OscFSK {
public:
    /**
     * Get next period
     * @param bit bit value
     * @param period return period
     * @return bool whether to advance to the next bit
     */
    bool next(bool bit, uint16_t* period);

    /**
     * FSK ocillator constructor
     * 
     * @param freq_low bit 0 freq
     * @param freq_hi bit 1 freq
     * @param osc_phase_max max oscillator phase
     */
    OscFSK(uint16_t freq_low, uint16_t freq_hi, uint16_t osc_phase_max);

private:
    const uint16_t freq[2];
    const uint16_t osc_phase_max;
    int32_t osc_phase_current;
};
