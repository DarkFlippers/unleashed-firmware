#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <furi-hal-version.h>

#define GAP_MAC_ADDR_SIZE (6)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BleEventTypeConnected,
    BleEventTypeDisconnected,
    BleEventTypeStartAdvertising,
    BleEventTypeStopAdvertising,
    BleEventTypePinCodeShow,
    BleEventTypePinCodeVerify,
    BleEventTypeUpdateMTU,
} BleEventType;

typedef union {
    uint32_t pin_code;
    uint16_t max_packet_size;
} BleEventData;

typedef struct {
    BleEventType type;
    BleEventData data;
} BleEvent;

typedef bool(*BleEventCallback) (BleEvent event, void* context);

typedef enum {
    GapStateIdle,
    GapStateStartingAdv,
    GapStateAdvFast,
    GapStateAdvLowPower,
    GapStateConnected,
} GapState;

typedef enum {
    GapPairingPinCodeShow,
    GapPairingPinCodeVerifyYesNo,
} GapPairing;

typedef struct {
    uint16_t adv_service_uuid;
    uint16_t appearance_char;
    bool bonding_mode;
    GapPairing pairing_method;
    uint8_t mac_address[GAP_MAC_ADDR_SIZE];
    char adv_name[FURI_HAL_VERSION_DEVICE_NAME_LENGTH];
} GapConfig;

bool gap_init(GapConfig* config, BleEventCallback on_event_cb, void* context);

void gap_start_advertising();

void gap_stop_advertising();

GapState gap_get_state();

void gap_thread_stop();

#ifdef __cplusplus
}
#endif
