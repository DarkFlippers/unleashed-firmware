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

void gap_start_advertising();

void gap_stop_advertising();

GapState gap_get_state();

#ifdef __cplusplus
}
#endif
