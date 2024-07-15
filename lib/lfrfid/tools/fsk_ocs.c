#include "fsk_osc.h"
#include <stdlib.h>

struct FSKOsc {
    uint16_t freq[2];
    uint16_t osc_phase_max;
    int32_t osc_phase_current;

    uint32_t pulse;
};

FSKOsc* fsk_osc_alloc(uint32_t freq_low, uint32_t freq_hi, uint32_t osc_phase_max) {
    FSKOsc* osc = malloc(sizeof(FSKOsc));
    osc->freq[0] = freq_low;
    osc->freq[1] = freq_hi;
    osc->osc_phase_max = osc_phase_max;
    osc->osc_phase_current = 0;
    osc->pulse = 0;
    return osc;
}

void fsk_osc_free(FSKOsc* osc) {
    free(osc);
}

void fsk_osc_reset(FSKOsc* osc) {
    osc->osc_phase_current = 0;
    osc->pulse = 0;
}

bool fsk_osc_next(FSKOsc* osc, bool bit, uint32_t* period) {
    bool advance = false;
    *period = osc->freq[bit];
    osc->osc_phase_current += *period;

    if(osc->osc_phase_current > osc->osc_phase_max) {
        advance = true;
        osc->osc_phase_current -= osc->osc_phase_max;
    }

    return advance;
}

bool fsk_osc_next_half(FSKOsc* osc, bool bit, bool* level, uint32_t* duration) {
    bool advance = false;

    // if pulse is zero, we need to output high, otherwise we need to output low
    if(osc->pulse == 0) {
        uint32_t length;
        advance = fsk_osc_next(osc, bit, &length);
        *duration = length / 2;
        osc->pulse = *duration;
        *level = true;
    } else {
        // output low half and reset pulse
        *duration = osc->pulse;
        osc->pulse = 0;
        *level = false;
    }

    return advance;
}
