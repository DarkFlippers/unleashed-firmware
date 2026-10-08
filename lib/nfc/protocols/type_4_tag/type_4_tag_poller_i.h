#pragma once

#include "type_4_tag_poller.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_poller_i.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    Type4TagPollerStateIdle,
    Type4TagPollerStateRequestMode,
    Type4TagPollerStateDetectPlatform,
    Type4TagPollerStateSelectApplication,
    Type4TagPollerStateReadCapabilityContainer,
    Type4TagPollerStateReadNdefMessage,
    Type4TagPollerStateCreateApplication,
    Type4TagPollerStateCreateCapabilityContainer,
    Type4TagPollerStateCreateNdefMessage,
    Type4TagPollerStateWriteNdefMessage,
    Type4TagPollerStateFailed,
    Type4TagPollerStateSuccess,

    Type4TagPollerStateNum,
} Type4TagPollerState;

struct Type4TagPoller {
    Iso14443_4aPoller* iso14443_4a_poller;
    Type4TagPollerState state;
    Type4TagPollerMode mode;
    Type4TagError error;
    Type4TagData* data;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;
    Type4TagPollerEventData type_4_tag_event_data;
    Type4TagPollerEvent type_4_tag_event;
    NfcGenericEvent general_event;
    NfcGenericCallback callback;
    void* context;
};

Type4TagError type_4_tag_poller_detect_platform(Type4TagPoller* instance);

Type4TagError type_4_tag_poller_select_app(Type4TagPoller* instance);

Type4TagError type_4_tag_poller_read_cc(Type4TagPoller* instance);

Type4TagError type_4_tag_poller_read_ndef(Type4TagPoller* instance);

Type4TagError type_4_tag_poller_create_app(Type4TagPoller* instance);

Type4TagError type_4_tag_poller_create_cc(Type4TagPoller* instance);

Type4TagError type_4_tag_poller_create_ndef(Type4TagPoller* instance);

Type4TagError type_4_tag_poller_write_ndef(Type4TagPoller* instance);

#ifdef __cplusplus
}
#endif
