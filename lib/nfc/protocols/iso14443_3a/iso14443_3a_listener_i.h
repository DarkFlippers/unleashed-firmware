#pragma once

#include "iso14443_3a_listener.h"
#include <nfc/protocols/nfc_generic_event.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    Iso14443_3aListenerStateIdle,
    Iso14443_3aListenerStateActive,
} Iso14443_3aListenerState;

struct Iso14443_3aListener {
    Nfc* nfc;
    Iso14443_3aData* data;
    Iso14443_3aListenerState state;

    BitBuffer* tx_buffer;

    NfcGenericEvent generic_event;
    Iso14443_3aListenerEvent iso14443_3a_event;
    Iso14443_3aListenerEventData iso14443_3a_event_data;
    NfcGenericCallback callback;
    void* context;
};

Iso14443_3aError
    iso14443_3a_listener_tx(Iso14443_3aListener* instance, const BitBuffer* tx_buffer);

Iso14443_3aError iso14443_3a_listener_tx_with_custom_parity(
    Iso14443_3aListener* instance,
    const BitBuffer* tx_buffer);

Iso14443_3aError iso14443_3a_listener_send_standard_frame(
    Iso14443_3aListener* instance,
    const BitBuffer* tx_buffer);

#ifdef __cplusplus
}
#endif
