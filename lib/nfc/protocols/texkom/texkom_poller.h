#pragma once

#include "texkom.h"

#include <lib/nfc/nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TexkomPoller TexkomPoller;

typedef enum {
    TexkomPollerEventTypeReady, /**< A frame was captured and decoded. */
} TexkomPollerEventType;

typedef struct {
    TexkomPollerEventType type;
} TexkomPollerEvent;

#ifdef __cplusplus
}
#endif
