#pragma once

#include "mf_ultralight.h"
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MfUltralightPoller MfUltralightPoller;

typedef enum {
    MfUltralightPollerEventTypeAuthRequest,
    MfUltralightPollerEventTypeAuthSuccess,
    MfUltralightPollerEventTypeAuthFailed,
    MfUltralightPollerEventTypeReadSuccess,
    MfUltralightPollerEventTypeReadFailed,
} MfUltralightPollerEventType;

typedef struct {
    MfUltralightAuthPassword password;
    MfUltralightAuthPack pack;
    bool auth_success;
    bool skip_auth;
} MfUltralightPollerAuthContext;

typedef struct {
    union {
        MfUltralightPollerAuthContext auth_context;
        MfUltralightError error;
    };
} MfUltralightPollerEventData;

typedef struct {
    MfUltralightPollerEventType type;
    MfUltralightPollerEventData* data;
} MfUltralightPollerEvent;

#ifdef __cplusplus
}
#endif
