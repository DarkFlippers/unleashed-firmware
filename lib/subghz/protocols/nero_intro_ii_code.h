#pragma once

#include <stdbool.h>
#include <stdint.h>

#define NERO_INTRO_II_SYMBOLS   15
#define NERO_INTRO_II_BITS      105
#define NERO_INTRO_II_MAX_CHIPS 1260
#define NERO_INTRO_II_PATHS     35

/* Canonical line-code symbols, not identified application-layer fields. */
typedef struct {
    uint8_t symbols[NERO_INTRO_II_SYMBOLS];
    uint8_t corrections;
    bool ambiguous;
} NeroIntroIIFrame;

typedef struct {
    uint8_t cost;
    bool ambiguous;
    uint8_t symbols[NERO_INTRO_II_SYMBOLS];
} NeroIntroIIPath;

/* Caller-owned workspace keeps the receive worker's stack bounded. */
typedef struct {
    uint64_t sync;
    uint16_t count;
    uint8_t sync_count;
    uint8_t chips[NERO_INTRO_II_MAX_CHIPS];
    NeroIntroIIPath paths[2][NERO_INTRO_II_PATHS];
} NeroIntroIICode;

void nero_intro_ii_code_reset(NeroIntroIICode* instance);
bool nero_intro_ii_code_feed(
    NeroIntroIICode* instance,
    bool level,
    uint32_t duration,
    NeroIntroIIFrame* frame);
