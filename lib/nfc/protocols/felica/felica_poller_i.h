#pragma once

#include "felica_poller.h"

#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FELICA_POLLER_MAX_BUFFER_SIZE (256U)

#define FELICA_POLLER_POLLING_FWT (200000U)

#define FELICA_POLLER_CMD_POLLING_REQ_CODE (0x00U)
#define FELICA_POLLER_CMD_POLLING_RESP_CODE (0x01U)

typedef enum {
    FelicaPollerStateIdle,
    FelicaPollerStateActivated,
} FelicaPollerState;

struct FelicaPoller {
    Nfc* nfc;
    FelicaPollerState state;
    FelicaData* data;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;

    NfcGenericEvent general_event;
    FelicaPollerEvent felica_event;
    FelicaPollerEventData felica_event_data;
    NfcGenericCallback callback;
    void* context;
};

typedef struct {
    uint16_t system_code;
    uint8_t request_code;
    uint8_t time_slot;
} FelicaPollerPollingCommand;

typedef struct {
    FelicaIDm idm;
    FelicaPMm pmm;
    uint8_t request_data[2];
} FelicaPollerPollingResponse;

const FelicaData* felica_poller_get_data(FelicaPoller* instance);

FelicaError felica_poller_polling(
    FelicaPoller* instance,
    const FelicaPollerPollingCommand* cmd,
    FelicaPollerPollingResponse* resp);

#ifdef __cplusplus
}
#endif
