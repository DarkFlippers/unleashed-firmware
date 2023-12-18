#pragma once

#include <nfc/protocols/iso14443_3a/iso14443_3a_poller_i.h>
#include <nfc/helpers/iso14443_4_layer.h>

#include "iso14443_4a_poller.h"
#include "iso14443_4a_i.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISO14443_4A_POLLER_ATS_FWT_FC (40000)

typedef enum {
    Iso14443_4aPollerStateIdle,
    Iso14443_4aPollerStateReadAts,
    Iso14443_4aPollerStateError,
    Iso14443_4aPollerStateReady,

    Iso14443_4aPollerStateNum,
} Iso14443_4aPollerState;

typedef enum {
    Iso14443_4aPollerSessionStateIdle,
    Iso14443_4aPollerSessionStateActive,
    Iso14443_4aPollerSessionStateStopRequest,
} Iso14443_4aPollerSessionState;

struct Iso14443_4aPoller {
    Iso14443_3aPoller* iso14443_3a_poller;
    Iso14443_4aPollerState poller_state;
    Iso14443_4aPollerSessionState session_state;
    Iso14443_4aError error;
    Iso14443_4aData* data;
    Iso14443_4Layer* iso14443_4_layer;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;
    Iso14443_4aPollerEventData iso14443_4a_event_data;
    Iso14443_4aPollerEvent iso14443_4a_event;
    NfcGenericEvent general_event;
    NfcGenericCallback callback;
    void* context;
};

const Iso14443_4aData* iso14443_4a_poller_get_data(Iso14443_4aPoller* instance);

#ifdef __cplusplus
}
#endif
