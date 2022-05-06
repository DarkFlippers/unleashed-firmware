#include "manchester_encoder.h"
#include <stdio.h>

void manchester_encoder_reset(ManchesterEncoderState* state) {
    state->step = 0;
}

bool manchester_encoder_advance(
    ManchesterEncoderState* state,
    const bool curr_bit,
    ManchesterEncoderResult* result) {
    bool advance = false;
    switch(state->step) {
    case 0:
        state->prev_bit = curr_bit;
        if(state->prev_bit) {
            *result = ManchesterEncoderResultShortLow;
        } else {
            *result = ManchesterEncoderResultShortHigh;
        }
        state->step = 1;
        advance = true;
        break;
    case 1:
        *result = (state->prev_bit << 1) + curr_bit;
        if(curr_bit == state->prev_bit) {
            state->step = 2;
        } else {
            state->prev_bit = curr_bit;
            advance = true;
        }
        break;
    case 2:
        if(curr_bit) {
            *result = ManchesterEncoderResultShortLow;
        } else {
            *result = ManchesterEncoderResultShortHigh;
        }
        state->prev_bit = curr_bit;
        state->step = 1;
        advance = true;
        break;
    default:
        printf("DO CRASH HERE\r\n");
        // furi_crash
        break;
    }
    return advance;
}

ManchesterEncoderResult manchester_encoder_finish(ManchesterEncoderState* state) {
    state->step = 0;
    return (state->prev_bit << 1) + state->prev_bit;
}
