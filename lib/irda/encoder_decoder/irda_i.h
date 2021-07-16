#pragma once
#include "irda.h"
#include <stddef.h>

typedef struct {
    uint32_t silence_time;
    uint16_t preamble_mark;
    uint16_t preamble_space;
    uint16_t bit1_mark;
    uint16_t bit1_space;
    uint16_t bit0_mark;
    uint16_t bit0_space;
    float    preamble_tolerance;
    uint32_t bit_tolerance;
} IrdaTimings;

typedef void* (*IrdaAlloc) (void);
typedef IrdaMessage* (*IrdaDecode) (void* ctx, bool level, uint32_t duration);
typedef void (*IrdaReset) (void*);
typedef void (*IrdaFree) (void*);

typedef void (*IrdaEncoderReset)(void* encoder, const IrdaMessage* message);
typedef IrdaStatus (*IrdaEncode)(void* encoder, uint32_t* out, bool* polarity);
typedef IrdaTimings (*IrdaTimingsGet)(void);

static inline uint8_t reverse(uint8_t value) {
    uint8_t reverse_value = 0;
    for (int i = 0; i < 8; ++i) {
        reverse_value |= (value & (0x01 << i)) ? 1 << (7 - i) : 0;
    }

    return reverse_value;
}

