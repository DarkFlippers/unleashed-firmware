#include "manchester_decoder.h"
#include <stdint.h>

static const uint8_t transitions[] = {0b00000001, 0b10010001, 0b10011011, 0b11111011};
static const ManchesterState manchester_reset_state = ManchesterStateMid1;

bool manchester_advance(
    ManchesterState state,
    ManchesterEvent event,
    ManchesterState* next_state,
    bool* data) {
    bool result = false;
    ManchesterState new_state;

    if(event == ManchesterEventReset) {
        new_state = manchester_reset_state;
    } else {
        new_state = transitions[state] >> event & 0x3;
        if(new_state == state) {
            new_state = manchester_reset_state;
        } else {
            if(new_state == ManchesterStateMid0) {
                if(data) *data = false;
                result = true;
            } else if(new_state == ManchesterStateMid1) {
                if(data) *data = true;
                result = true;
            }
        }
    }

    *next_state = new_state;
    return result;
}
