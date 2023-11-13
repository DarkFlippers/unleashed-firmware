#pragma once

#include "st25tb.h"
#include <lib/nfc/nfc.h>

#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct St25tbPoller St25tbPoller;

typedef enum {
    St25tbPollerEventTypeError,
    St25tbPollerEventTypeReady,
} St25tbPollerEventType;

typedef struct {
    St25tbError error;
} St25tbPollerEventData;

typedef struct {
    St25tbPollerEventType type;
    St25tbPollerEventData* data;
} St25tbPollerEvent;

#ifdef __cplusplus
}
#endif
