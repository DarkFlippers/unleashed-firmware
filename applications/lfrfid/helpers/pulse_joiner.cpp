#include "pulse_joiner.h"
#include <furi.h>

bool PulseJoiner::push_pulse(bool polarity, uint16_t period, uint16_t pulse) {
    bool result = false;
    furi_check((pulse_index + 1) < pulse_max);

    if(polarity == false && pulse_index == 0) {
        // first negative pulse is ommited

    } else {
        pulses[pulse_index].polarity = polarity;
        pulses[pulse_index].time = pulse;
        pulse_index++;
    }

    if(period > pulse) {
        pulses[pulse_index].polarity = !polarity;
        pulses[pulse_index].time = period - pulse;
        pulse_index++;
    }

    if(pulse_index >= 4) {
        // we know that first pulse is always high
        // so we wait 2 edges, hi2low and next low2hi

        uint8_t edges_count = 0;
        bool last_polarity = pulses[0].polarity;

        for(uint8_t i = 1; i < pulse_index; i++) {
            if(pulses[i].polarity != last_polarity) {
                edges_count++;
                last_polarity = pulses[i].polarity;
            }
        }

        if(edges_count >= 2) {
            result = true;
        }
    }

    return result;
}

void PulseJoiner::pop_pulse(uint16_t* period, uint16_t* pulse) {
    furi_check(pulse_index <= (pulse_max + 1));

    uint16_t tmp_period = 0;
    uint16_t tmp_pulse = 0;
    uint8_t edges_count = 0;
    bool last_polarity = pulses[0].polarity;
    uint8_t next_fist_pulse = 0;

    for(uint8_t i = 0; i < pulse_max; i++) {
        // count edges
        if(pulses[i].polarity != last_polarity) {
            edges_count++;
            last_polarity = pulses[i].polarity;
        }

        // wait for 2 edges
        if(edges_count == 2) {
            next_fist_pulse = i;
            break;
        }

        // sum pulse time
        if(pulses[i].polarity) {
            tmp_period += pulses[i].time;
            tmp_pulse += pulses[i].time;
        } else {
            tmp_period += pulses[i].time;
        }
        pulse_index--;
    }

    *period = tmp_period;
    *pulse = tmp_pulse;

    // remove counted periods and shift data
    for(uint8_t i = 0; i < pulse_max; i++) {
        if((next_fist_pulse + i) < pulse_max) {
            pulses[i].polarity = pulses[next_fist_pulse + i].polarity;
            pulses[i].time = pulses[next_fist_pulse + i].time;
        } else {
            break;
        }
    }
}

PulseJoiner::PulseJoiner() {
    for(uint8_t i = 0; i < pulse_max; i++) {
        pulses[i] = {false, 0};
    }
}
