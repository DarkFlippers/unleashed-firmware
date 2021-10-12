#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BleEventTypeConnected,
    BleEventTypeDisconnected,
    BleEventTypeStartAdvertising,
    BleEventTypeStopAdvertising,
    BleEventTypePinCodeShow,
} BleEventType;

typedef union {
    uint32_t pin_code;
} BleEventData;

typedef struct {
    BleEventType type;
    BleEventData data;
} BleEvent;

typedef void(*BleEventCallback) (BleEvent event, void* context);

typedef enum {
    GapStateIdle,
    GapStateAdvFast,
    GapStateAdvLowPower,
    GapStateConnected,
} GapState;

bool gap_init(BleEventCallback on_event_cb, void* context);

void gap_start_advertising();

void gap_stop_advertising();

GapState gap_get_state();

#ifdef __cplusplus
}
#endif
