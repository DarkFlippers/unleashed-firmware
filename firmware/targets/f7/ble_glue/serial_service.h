#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SERIAL_SVC_DATA_LEN_MAX (486)
#define SERIAL_SVC_CHAR_VALUE_LEN_MAX (243)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SerialServiceRpcStatusNotActive = 0UL,
    SerialServiceRpcStatusActive = 1UL,
} SerialServiceRpcStatus;

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

void serial_svc_start();

void serial_svc_set_callbacks(
    uint16_t buff_size,
    SerialServiceEventCallback callback,
    void* context);

void serial_svc_set_rpc_status(SerialServiceRpcStatus status);

void serial_svc_notify_buffer_is_empty();

void serial_svc_stop();

bool serial_svc_is_started();

bool serial_svc_update_tx(uint8_t* data, uint16_t data_len);

#ifdef __cplusplus
}
#endif
