#pragma once
#include "irda.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t min_split_time;
    uint32_t silence_time;
    uint16_t preamble_mark;
    uint16_t preamble_space;
    uint16_t bit1_mark;
    uint16_t bit1_space;
    uint16_t bit0_mark;
    uint16_t bit0_space;
    uint32_t preamble_tolerance;
    uint32_t bit_tolerance;
} IrdaTimings;

typedef struct {
    const char* name;
    uint8_t address_length;
    uint8_t command_length;
    uint32_t frequency;
    float duty_cycle;
} IrdaProtocolSpecification;

typedef const IrdaProtocolSpecification* (*IrdaGetProtocolSpec)(IrdaProtocol protocol);

typedef void* (*IrdaAlloc)(void);
typedef void (*IrdaFree)(void*);

typedef void (*IrdaDecoderReset)(void*);
typedef IrdaMessage* (*IrdaDecode)(void* ctx, bool level, uint32_t duration);
typedef IrdaMessage* (*IrdaDecoderCheckReady)(void*);

typedef void (*IrdaEncoderReset)(void* encoder, const IrdaMessage* message);
typedef IrdaStatus (*IrdaEncode)(void* encoder, uint32_t* out, bool* polarity);

static inline uint8_t reverse(uint8_t value) {
    uint8_t reverse_value = 0;
    for(int i = 0; i < 8; ++i) {
        reverse_value |= (value & (0x01 << i)) ? 1 << (7 - i) : 0;
    }

    return reverse_value;
}
