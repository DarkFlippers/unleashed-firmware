#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool prev_bit;
    uint8_t step;
} ManchesterEncoderState;

typedef enum {
    ManchesterEncoderResultShortLow = 0b00,
    ManchesterEncoderResultLongLow = 0b01,
    ManchesterEncoderResultLongHigh = 0b10,
    ManchesterEncoderResultShortHigh = 0b11,
} ManchesterEncoderResult;

void manchester_encoder_reset(ManchesterEncoderState* state);

bool manchester_encoder_advance(
    ManchesterEncoderState* state,
    const bool curr_bit,
    ManchesterEncoderResult* result);

ManchesterEncoderResult manchester_encoder_finish(ManchesterEncoderState* state);

#ifdef __cplusplus
}
#endif
