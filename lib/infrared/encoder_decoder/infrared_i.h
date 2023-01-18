#pragma once
#include "infrared.h"
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
} InfraredTimings;

typedef struct {
    const char* name;
    uint8_t address_length;
    uint8_t command_length;
    uint32_t frequency;
    float duty_cycle;
    size_t repeat_count;
} InfraredProtocolVariant;

typedef const InfraredProtocolVariant* (*InfraredGetProtocolVariant)(InfraredProtocol protocol);

typedef void* (*InfraredAlloc)(void);
typedef void (*InfraredFree)(void*);

typedef void (*InfraredDecoderReset)(void*);
typedef InfraredMessage* (*InfraredDecode)(void* ctx, bool level, uint32_t duration);
typedef InfraredMessage* (*InfraredDecoderCheckReady)(void*);

typedef void (*InfraredEncoderReset)(void* encoder, const InfraredMessage* message);
typedef InfraredStatus (*InfraredEncode)(void* encoder, uint32_t* out, bool* polarity);

static inline uint8_t reverse(uint8_t value) {
    uint8_t reverse_value = 0;
    for(int i = 0; i < 8; ++i) {
        reverse_value |= (value & (0x01 << i)) ? 1 << (7 - i) : 0;
    }

    return reverse_value;
}
