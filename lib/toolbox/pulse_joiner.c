#include "pulse_joiner.h"
#include <furi.h>

#define PULSE_MAX_COUNT 6

typedef struct {
    bool polarity;
    uint16_t time;
} Pulse;

struct PulseJoiner {
    size_t pulse_index;
    Pulse pulses[PULSE_MAX_COUNT];
};

PulseJoiner* pulse_joiner_alloc() {
    PulseJoiner* pulse_joiner = malloc(sizeof(PulseJoiner));

    pulse_joiner->pulse_index = 0;
    for(uint8_t i = 0; i < PULSE_MAX_COUNT; i++) {
        pulse_joiner->pulses[i].polarity = false;
        pulse_joiner->pulses[i].time = 0;
    }

    return pulse_joiner;
}

void pulse_joiner_free(PulseJoiner* pulse_joiner) {
    free(pulse_joiner);
}

bool pulse_joiner_push_pulse(PulseJoiner* pulse_joiner, bool polarity, size_t period, size_t pulse) {
    bool result = false;
    furi_check((pulse_joiner->pulse_index + 1) < PULSE_MAX_COUNT);

    if(polarity == false && pulse_joiner->pulse_index == 0) {
        // first negative pulse is omitted

    } else {
        pulse_joiner->pulses[pulse_joiner->pulse_index].polarity = polarity;
        pulse_joiner->pulses[pulse_joiner->pulse_index].time = pulse;
        pulse_joiner->pulse_index++;
    }

    if(period > pulse) {
        pulse_joiner->pulses[pulse_joiner->pulse_index].polarity = !polarity;
        pulse_joiner->pulses[pulse_joiner->pulse_index].time = period - pulse;
        pulse_joiner->pulse_index++;
    }

    if(pulse_joiner->pulse_index >= 4) {
        // we know that first pulse is always high
        // so we wait 2 edges, hi2low and next low2hi

        uint8_t edges_count = 0;
        bool last_polarity = pulse_joiner->pulses[0].polarity;

        for(uint8_t i = 1; i < pulse_joiner->pulse_index; i++) {
            if(pulse_joiner->pulses[i].polarity != last_polarity) {
                edges_count++;
                last_polarity = pulse_joiner->pulses[i].polarity;
            }
        }

        if(edges_count >= 2) {
            result = true;
        }
    }

    return result;
}

void pulse_joiner_pop_pulse(PulseJoiner* pulse_joiner, size_t* period, size_t* pulse) {
    furi_check(pulse_joiner->pulse_index <= (PULSE_MAX_COUNT + 1));

    uint16_t tmp_period = 0;
    uint16_t tmp_pulse = 0;
    uint8_t edges_count = 0;
    bool last_polarity = pulse_joiner->pulses[0].polarity;
    uint8_t next_fist_pulse = 0;

    for(uint8_t i = 0; i < PULSE_MAX_COUNT; i++) {
        // count edges
        if(pulse_joiner->pulses[i].polarity != last_polarity) {
            edges_count++;
            last_polarity = pulse_joiner->pulses[i].polarity;
        }

        // wait for 2 edges
        if(edges_count == 2) {
            next_fist_pulse = i;
            break;
        }

        // sum pulse time
        if(pulse_joiner->pulses[i].polarity) {
            tmp_period += pulse_joiner->pulses[i].time;
            tmp_pulse += pulse_joiner->pulses[i].time;
        } else {
            tmp_period += pulse_joiner->pulses[i].time;
        }
        pulse_joiner->pulse_index--;
    }

    *period = tmp_period;
    *pulse = tmp_pulse;

    // remove counted periods and shift data
    for(uint8_t i = 0; i < PULSE_MAX_COUNT; i++) {
        if((next_fist_pulse + i) < PULSE_MAX_COUNT) {
            pulse_joiner->pulses[i].polarity = pulse_joiner->pulses[next_fist_pulse + i].polarity;
            pulse_joiner->pulses[i].time = pulse_joiner->pulses[next_fist_pulse + i].time;
        } else {
            break;
        }
    }
}