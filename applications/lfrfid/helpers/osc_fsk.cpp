#include "osc_fsk.h"

OscFSK::OscFSK(uint16_t _freq_low, uint16_t _freq_hi, uint16_t _osc_phase_max)
    : freq{_freq_low, _freq_hi}
    , osc_phase_max(_osc_phase_max) {
    osc_phase_current = 0;
}

bool OscFSK::next(bool bit, uint16_t* period) {
    bool advance = false;
    *period = freq[bit];
    osc_phase_current += *period;

    if(osc_phase_current > osc_phase_max) {
        advance = true;
        osc_phase_current -= osc_phase_max;
    }

    return advance;
}
