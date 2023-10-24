#pragma once

#include "slix.h"

#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SlixPoller SlixPoller;

typedef enum {
    SlixPollerEventTypeError,
    SlixPollerEventTypeReady,
} SlixPollerEventType;

typedef struct {
    SlixError error;
} SlixPollerEventData;

typedef struct {
    SlixPollerEventType type;
    SlixPollerEventData* data;
} SlixPollerEvent;

#ifdef __cplusplus
}
#endif
