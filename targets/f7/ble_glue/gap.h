#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <furi_hal_version.h>

#define GAP_MAC_ADDR_SIZE (6)

/*
 * GAP helpers - background thread that handles BLE GAP events and advertising.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GapEventTypeConnected,
    GapEventTypeDisconnected,
    GapEventTypeStartAdvertising,
    GapEventTypeStopAdvertising,
    GapEventTypePinCodeShow,
    GapEventTypePinCodeVerify,
    GapEventTypeUpdateMTU,
    GapEventTypeBeaconStart,
    GapEventTypeBeaconStop,
} GapEventType;

typedef union {
    uint32_t pin_code;
    uint16_t max_packet_size;
} GapEventData;

typedef struct {
    GapEventType type;
    GapEventData data;
} GapEvent;

typedef bool (*GapEventCallback)(GapEvent event, void* context);

typedef enum {
    GapStateUninitialized,
    GapStateIdle,
    GapStateStartingAdv,
    GapStateAdvFast,
    GapStateAdvLowPower,
    GapStateConnected,
} GapState;

typedef enum {
    GapPairingNone,
    GapPairingPinCodeShow,
    GapPairingPinCodeVerifyYesNo,
} GapPairing;

typedef struct {
    uint16_t conn_interval;
    uint16_t slave_latency;
    uint16_t supervisor_timeout;
} GapConnectionParams;

typedef struct {
    uint16_t conn_int_min;
    uint16_t conn_int_max;
    uint16_t slave_latency;
    uint16_t supervisor_timeout;
} GapConnectionParamsRequest;

typedef struct {
    uint16_t adv_service_uuid;
    uint16_t appearance_char;
    bool bonding_mode;
    GapPairing pairing_method;
    uint8_t mac_address[GAP_MAC_ADDR_SIZE];
    char adv_name[FURI_HAL_VERSION_DEVICE_NAME_LENGTH];
    GapConnectionParamsRequest conn_param;
} GapConfig;

bool gap_init(GapConfig* config, GapEventCallback on_event_cb, void* context);

void gap_start_advertising(void);

void gap_stop_advertising(void);

GapState gap_get_state(void);

void gap_thread_stop(void);

void gap_emit_ble_beacon_status_event(bool active);

#ifdef __cplusplus
}
#endif
