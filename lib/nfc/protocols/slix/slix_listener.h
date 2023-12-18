#pragma once

#include <lib/nfc/protocols/iso15693_3/iso15693_3_listener.h>

#include "slix.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SlixListener SlixListener;

typedef enum {
    SlixListenerEventTypeFieldOff,
    SlixListenerEventTypeCustomCommand,
} SlixListenerEventType;

typedef struct {
    BitBuffer* buffer;
} SlixListenerEventData;

typedef struct {
    SlixListenerEventType type;
    SlixListenerEventData* data;
} SlixListenerEvent;

#ifdef __cplusplus
}
#endif
