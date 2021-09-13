#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GapStateIdle,
    GapStateAdvFast,
    GapStateAdvLowPower,
    GapStateConnected,
} GapState;

bool gap_init();

GapState gap_get_status();

#ifdef __cplusplus
}
#endif
