#pragma once

#include <nfc/protocols/iso15693_3/iso15693_3_poller_i.h>

#include "slix_poller.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SlixPollerStateIdle,
    SlixPollerStateGetNxpSysInfo,
    SlixPollerStateReadSignature,
    SlixPollerStateReady,
    SlixPollerStateError,
    SlixPollerStateNum,
} SlixPollerState;

struct SlixPoller {
    Iso15693_3Poller* iso15693_3_poller;
    SlixData* data;
    SlixPollerState poller_state;
    SlixError error;

    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;

    SlixPollerEventData slix_event_data;
    SlixPollerEvent slix_event;
    NfcGenericEvent general_event;
    NfcGenericCallback callback;
    void* context;
};

#ifdef __cplusplus
}
#endif
