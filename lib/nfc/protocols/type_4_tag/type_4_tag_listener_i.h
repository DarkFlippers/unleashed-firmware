#pragma once

#include "type_4_tag_listener.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_listener_i.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    Type4TagListenerStateIdle,
    Type4TagListenerStateSelectedPicc,
    Type4TagListenerStateSelectedApplication,
    Type4TagListenerStateSelectedCapabilityContainer,
    Type4TagListenerStateSelectedNdefMessage,
} Type4TagListenerState;

struct Type4TagListener {
    Iso14443_4aListener* iso14443_4a_listener;
    Type4TagData* data;
    Type4TagListenerState state;

    BitBuffer* tx_buffer;

    NfcGenericEvent generic_event;
    Type4TagListenerEvent type_4_tag_event;
    Type4TagListenerEventData type_4_tag_event_data;
    NfcGenericCallback callback;
    void* context;
};

Type4TagError
    type_4_tag_listener_handle_apdu(Type4TagListener* instance, const BitBuffer* rx_buffer);

#ifdef __cplusplus
}
#endif
