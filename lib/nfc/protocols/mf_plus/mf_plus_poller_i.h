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

// Result of the active SAK-0x20 disambiguation probe. AN10833 gives no passive way to separate a
// Plus in SL0 from one in SL3, nor a Plus from a DESFire (both answer SAK 0x20 + ATS), so the
// poller sends a command and classifies the reply.
typedef enum {
    MfPlusProbeResultSl0, // answered 0x09: still in personalization (SL0)
    MfPlusProbeResultSl3, // answered as a Plus but not SL0
    MfPlusProbeResultNotPlus, // DESFire / unsupported: let the poller chain continue
} MfPlusProbeResult;

MfPlusError mf_plus_process_error(Iso14443_4aError error);

// Sends WritePerso to an intentionally invalid block and classifies the reply (mirrors PM3
// `hf mfp info`). Returns MfPlusErrorNone with *result set on a clean exchange, an error otherwise.
MfPlusError mf_plus_poller_probe_security_level(MfPlusPoller* instance, MfPlusProbeResult* result);

MfPlusPoller* mf_plus_poller_alloc(Iso14443_4aPoller* iso14443_4a_poller);

void mf_plus_poller_free(MfPlusPoller* instance);

#ifdef __cplusplus
}
#endif
