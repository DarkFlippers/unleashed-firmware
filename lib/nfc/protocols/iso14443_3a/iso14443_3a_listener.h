#pragma once

#include "iso14443_3a.h"
#include <nfc/nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso14443_3aListener Iso14443_3aListener;

typedef enum {
    Iso14443_3aListenerEventTypeFieldOff,
    Iso14443_3aListenerEventTypeHalted,

    Iso14443_3aListenerEventTypeReceivedStandardFrame,
    Iso14443_3aListenerEventTypeReceivedData,
} Iso14443_3aListenerEventType;

typedef struct {
    BitBuffer* buffer;
} Iso14443_3aListenerEventData;

typedef struct {
    Iso14443_3aListenerEventType type;
    Iso14443_3aListenerEventData* data;
} Iso14443_3aListenerEvent;

#ifdef __cplusplus
}
#endif
