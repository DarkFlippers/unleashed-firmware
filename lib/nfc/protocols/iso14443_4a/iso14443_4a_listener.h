#pragma once

#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_listener.h>

#include "iso14443_4a.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso14443_4aListener Iso14443_4aListener;

typedef enum {
    Iso14443_4aListenerEventTypeHalted,
    Iso14443_4aListenerEventTypeFieldOff,
    Iso14443_4aListenerEventTypeReceivedData,
} Iso14443_4aListenerEventType;

typedef struct {
    BitBuffer* buffer;
} Iso14443_4aListenerEventData;

typedef struct {
    Iso14443_4aListenerEventType type;
    Iso14443_4aListenerEventData* data;
} Iso14443_4aListenerEvent;

#ifdef __cplusplus
}
#endif
