#pragma once

#include "protocols/st25tb/st25tb.h"
#include "st25tb_poller.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ST25TB_POLLER_MAX_BUFFER_SIZE (16U)

typedef enum {
    St25tbPollerStateIdle,
    St25tbPollerStateInitiateInProgress,
    St25tbPollerStateInitiateFailed,
    St25tbPollerStateActivationInProgress,
    St25tbPollerStateActivationFailed,
    St25tbPollerStateActivated,
} St25tbPollerState;

struct St25tbPoller {
    Nfc* nfc;
    St25tbPollerState state;
    St25tbData* data;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;

    NfcGenericEvent general_event;
    St25tbPollerEvent st25tb_event;
    St25tbPollerEventData st25tb_event_data;
    NfcGenericCallback callback;
    void* context;
};

const St25tbData* st25tb_poller_get_data(St25tbPoller* instance);

#ifdef __cplusplus
}
#endif
