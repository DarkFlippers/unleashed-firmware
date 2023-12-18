#pragma once

#include "mf_ultralight.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MfUltralightListener MfUltralightListener;

typedef enum {
    MfUltralightListenerEventTypeAuth,
} MfUltralightListenerEventType;

typedef struct {
    union {
        MfUltralightAuthPassword password;
    };
} MfUltralightListenerEventData;

typedef struct {
    MfUltralightListenerEventType type;
    MfUltralightListenerEventData* data;
} MfUltralightListenerEvent;

#ifdef __cplusplus
}
#endif
