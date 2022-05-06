#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ManchesterEventShortLow = 0,
    ManchesterEventShortHigh = 2,
    ManchesterEventLongLow = 4,
    ManchesterEventLongHigh = 6,
    ManchesterEventReset = 8
} ManchesterEvent;

typedef enum {
    ManchesterStateStart1 = 0,
    ManchesterStateMid1 = 1,
    ManchesterStateMid0 = 2,
    ManchesterStateStart0 = 3
} ManchesterState;

bool manchester_advance(
    ManchesterState state,
    ManchesterEvent event,
    ManchesterState* next_state,
    bool* data);

#ifdef __cplusplus
}
#endif
