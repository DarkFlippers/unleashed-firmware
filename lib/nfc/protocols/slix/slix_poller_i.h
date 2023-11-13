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

SlixError slix_poller_send_frame(
    SlixPoller* instance,
    const BitBuffer* tx_data,
    BitBuffer* rx_data,
    uint32_t fwt);

SlixError slix_poller_async_get_nxp_system_info(SlixPoller* instance, SlixSystemInfo* data);

SlixError slix_poller_async_read_signature(SlixPoller* instance, SlixSignature* data);

#ifdef __cplusplus
}
#endif
