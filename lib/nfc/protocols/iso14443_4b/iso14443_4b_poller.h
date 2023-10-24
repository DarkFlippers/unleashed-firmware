#pragma once

#include <lib/nfc/protocols/iso14443_3b/iso14443_3b_poller.h>

#include "iso14443_4b.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso14443_4bPoller Iso14443_4bPoller;

typedef enum {
    Iso14443_4bPollerEventTypeError,
    Iso14443_4bPollerEventTypeReady,
} Iso14443_4bPollerEventType;

typedef struct {
    Iso14443_4bError error;
} Iso14443_4bPollerEventData;

typedef struct {
    Iso14443_4bPollerEventType type;
    Iso14443_4bPollerEventData* data;
} Iso14443_4bPollerEvent;

#ifdef __cplusplus
}
#endif
