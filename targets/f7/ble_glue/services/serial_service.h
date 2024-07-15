#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Serial service. Implements RPC over BLE, with flow control.
 */

#define BLE_SVC_SERIAL_DATA_LEN_MAX       (486)
#define BLE_SVC_SERIAL_CHAR_VALUE_LEN_MAX (243)

typedef enum {
    SerialServiceEventTypeDataReceived,
    SerialServiceEventTypeDataSent,
    SerialServiceEventTypesBleResetRequest,
} SerialServiceEventType;

typedef struct {
    uint8_t* buffer;
    uint16_t size;
} SerialServiceData;

typedef struct {
    SerialServiceEventType event;
    SerialServiceData data;
} SerialServiceEvent;

typedef uint16_t (*SerialServiceEventCallback)(SerialServiceEvent event, void* context);

typedef struct BleServiceSerial BleServiceSerial;

BleServiceSerial* ble_svc_serial_start(void);

void ble_svc_serial_stop(BleServiceSerial* service);

void ble_svc_serial_set_callbacks(
    BleServiceSerial* service,
    uint16_t buff_size,
    SerialServiceEventCallback callback,
    void* context);

void ble_svc_serial_set_rpc_active(BleServiceSerial* service, bool active);

void ble_svc_serial_notify_buffer_is_empty(BleServiceSerial* service);

bool ble_svc_serial_update_tx(BleServiceSerial* service, uint8_t* data, uint16_t data_len);

#ifdef __cplusplus
}
#endif
