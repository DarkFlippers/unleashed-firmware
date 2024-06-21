#pragma once

#include "mf_plus_poller.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_poller_i.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MF_PLUS_FWT_FC (60000)

typedef enum {
    MfPlusCardStateDetected,
    MfPlusCardStateLost,
} MfPlusCardState;

typedef enum {
    MfPlusPollerStateIdle,
    MfPlusPollerStateReadVersion,
    MfPlusPollerStateParseVersion,
    MfPlusPollerStateParseIso4,
    MfPlusPollerStateReadFailed,
    MfPlusPollerStateReadSuccess,

    MfPlusPollerStateNum,
} MfPlusPollerState;

struct MfPlusPoller {
    Iso14443_4aPoller* iso14443_4a_poller;

    MfPlusData* data;
    MfPlusPollerState state;

    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;
    BitBuffer* input_buffer;
    BitBuffer* result_buffer;

    MfPlusError error;
    NfcGenericEvent general_event;
    MfPlusPollerEvent mfp_event;
    MfPlusPollerEventData mfp_event_data;
    NfcGenericCallback callback;
    void* context;
};

MfPlusError mf_plus_process_error(Iso14443_4aError error);

MfPlusPoller* mf_plus_poller_alloc(Iso14443_4aPoller* iso14443_4a_poller);

void mf_plus_poller_free(MfPlusPoller* instance);

#ifdef __cplusplus
}
#endif
