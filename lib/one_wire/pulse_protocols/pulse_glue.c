#include "pulse_glue.h"

struct PulseGlue {
    int32_t hi_period;
    int32_t low_period;
    int32_t next_hi_period;
};

PulseGlue* pulse_glue_alloc() {
    PulseGlue* pulse_glue = malloc(sizeof(PulseGlue));
    pulse_glue_reset(pulse_glue);
    return pulse_glue;
}

void pulse_glue_free(PulseGlue* pulse_glue) {
    free(pulse_glue);
}

void pulse_glue_reset(PulseGlue* pulse_glue) {
    pulse_glue->hi_period = 0;
    pulse_glue->low_period = 0;
    pulse_glue->next_hi_period = 0;
}

bool pulse_glue_push(PulseGlue* pulse_glue, bool polarity, uint32_t length) {
    bool pop_ready = false;
    if(polarity) {
        if(pulse_glue->low_period == 0) {
            // stage 1, accumulate hi period
            pulse_glue->hi_period += length;
        } else {
            // stage 3, accumulate next hi period and be ready for pulse_glue_pop
            pulse_glue->next_hi_period = length;

            // data is ready
            pop_ready = true;
        }
    } else {
        if(pulse_glue->hi_period != 0) {
            // stage 2, accumulate low period
            pulse_glue->low_period += length;
        }
    }

    return pop_ready;
}

void pulse_glue_pop(PulseGlue* pulse_glue, uint32_t* length, uint32_t* period) {
    *length = pulse_glue->hi_period + pulse_glue->low_period;
    *period = pulse_glue->hi_period;

    pulse_glue->hi_period = pulse_glue->next_hi_period;
    pulse_glue->low_period = 0;
    pulse_glue->next_hi_period = 0;
}
