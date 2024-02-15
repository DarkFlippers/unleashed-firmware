#pragma once

#include "emv_poller.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_poller_i.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EmvPollerStateIdle,
    EmvPollerStateSelectPPSE,
    EmvPollerStateSelectApplication,
    EmvPollerStateGetProcessingOptions,
    EmvPollerStateReadFiles,
    EmvPollerStateReadExtra,
    EmvPollerStateReadFailed,
    EmvPollerStateReadSuccess,

    EmvPollerStateNum,
} EmvPollerState;

typedef enum {
    EmvPollerSessionStateIdle,
    EmvPollerSessionStateActive,
    EmvPollerSessionStateStopRequest,
} EmvPollerSessionState;

struct EmvPoller {
    Iso14443_4aPoller* iso14443_4a_poller;
    EmvPollerSessionState session_state;
    EmvPollerState state;
    EmvError error;
    EmvData* data;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;
    EmvPollerEventData emv_event_data;
    EmvPollerEvent emv_event;
    NfcGenericEvent general_event;
    NfcGenericCallback callback;
    uint16_t records_mask;
    void* context;
};

EmvError emv_process_error(Iso14443_4aError error);

const EmvData* emv_poller_get_data(EmvPoller* instance);

#ifdef __cplusplus
}
#endif
