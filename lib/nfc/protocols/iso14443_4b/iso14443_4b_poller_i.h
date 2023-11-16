#pragma once

#include <nfc/protocols/iso14443_3b/iso14443_3b_poller_i.h>
#include <nfc/helpers/iso14443_4_layer.h>

#include "iso14443_4b_poller.h"
#include "iso14443_4b_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    Iso14443_4bPollerStateIdle,
    Iso14443_4bPollerStateError,
    Iso14443_4bPollerStateReady,

    Iso14443_4bPollerStateNum,
} Iso14443_4bPollerState;

typedef enum {
    Iso14443_4bPollerSessionStateIdle,
    Iso14443_4bPollerSessionStateActive,
    Iso14443_4bPollerSessionStateStopRequest,
} Iso14443_4bPollerSessionState;

struct Iso14443_4bPoller {
    Iso14443_3bPoller* iso14443_3b_poller;
    Iso14443_4bPollerState poller_state;
    Iso14443_4bPollerSessionState session_state;
    Iso14443_4bError error;
    Iso14443_4bData* data;
    Iso14443_4Layer* iso14443_4_layer;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;
    Iso14443_4bPollerEventData iso14443_4b_event_data;
    Iso14443_4bPollerEvent iso14443_4b_event;
    NfcGenericEvent general_event;
    NfcGenericCallback callback;
    void* context;
};

const Iso14443_4bData* iso14443_4b_poller_get_data(Iso14443_4bPoller* instance);

#ifdef __cplusplus
}
#endif
