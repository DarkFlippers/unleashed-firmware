#pragma once

#include "iso14443_3b.h"
#include <lib/nfc/nfc.h>

#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso14443_3bPoller Iso14443_3bPoller;

typedef enum {
    Iso14443_3bPollerEventTypeError,
    Iso14443_3bPollerEventTypeReady,
} Iso14443_3bPollerEventType;

typedef struct {
    Iso14443_3bError error;
} Iso14443_3bPollerEventData;

typedef struct {
    Iso14443_3bPollerEventType type;
    Iso14443_3bPollerEventData* data;
} Iso14443_3bPollerEvent;

#ifdef __cplusplus
}
#endif
