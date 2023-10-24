#pragma once

#include "iso15693_3.h"

#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso15693_3Poller Iso15693_3Poller;

typedef enum {
    Iso15693_3PollerEventTypeError,
    Iso15693_3PollerEventTypeReady,
} Iso15693_3PollerEventType;

typedef struct {
    Iso15693_3Error error;
} Iso15693_3PollerEventData;

typedef struct {
    Iso15693_3PollerEventType type;
    Iso15693_3PollerEventData* data;
} Iso15693_3PollerEvent;

#ifdef __cplusplus
}
#endif
