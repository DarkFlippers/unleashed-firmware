#pragma once

#include "mf_classic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MfClassicListener MfClassicListener;

typedef enum {
    MfClassicListenerEventTypeAuthContextPartCollected,
    MfClassicListenerEventTypeAuthContextFullCollected,
} MfClassicListenerEventType;

typedef union {
    MfClassicAuthContext auth_context;
} MfClassicListenerEventData;

typedef struct {
    MfClassicListenerEventType type;
    MfClassicListenerEventData* data;
} MfClassicListenerEvent;

#ifdef __cplusplus
}
#endif
