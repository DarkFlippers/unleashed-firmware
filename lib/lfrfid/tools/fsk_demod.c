#include <furi.h>
#include "fsk_demod.h"

struct FSKDemod {
    uint32_t low_time;
    uint32_t low_pulses;
    uint32_t hi_time;
    uint32_t hi_pulses;

    bool invert;
    uint32_t mid_time;
    uint32_t time;
    uint32_t count;
    bool last_pulse;
};

FSKDemod*
    fsk_demod_alloc(uint32_t low_time, uint32_t low_pulses, uint32_t hi_time, uint32_t hi_pulses) {
    FSKDemod* demod = malloc(sizeof(FSKDemod));
    demod->invert = false;

    if(low_time > hi_time) {
        uint32_t tmp;
        tmp = hi_time;
        hi_time = low_time;
        low_time = tmp;

        tmp = hi_pulses;
        hi_pulses = low_pulses;
        low_pulses = tmp;

        demod->invert = true;
    }

    demod->low_time = low_time;
    demod->low_pulses = low_pulses;
    demod->hi_time = hi_time;
    demod->hi_pulses = hi_pulses;

    demod->mid_time = (hi_time - low_time) / 2 + low_time;
    demod->time = 0;
    demod->count = 0;
    demod->last_pulse = false;

    return demod;
}

void fsk_demod_free(FSKDemod* demod) {
    free(demod);
}

void fsk_demod_feed(FSKDemod* demod, bool polarity, uint32_t time, bool* value, uint32_t* count) {
    *count = 0;

    if(polarity) {
        // accumulate time
        demod->time = time;
    } else {
        demod->time += time;

        // check for valid pulse
        if(demod->time >= demod->low_time && demod->time < demod->hi_time) {
            bool pulse;

            if(demod->time < demod->mid_time) {
                pulse = false;
            } else {
                pulse = true;
            }

            demod->count++;

            // check for edge transition
            if(demod->last_pulse != pulse) {
                uint32_t data_count = demod->count + 1;

                if(demod->last_pulse) {
                    data_count /= demod->hi_pulses;
                    *value = !demod->invert;
                } else {
                    data_count /= demod->low_pulses;
                    *value = demod->invert;
                }

                *count = data_count;
                demod->count = 0;
                demod->last_pulse = pulse;
            }
        } else {
            demod->count = 0;
        }
    }
}
