#pragma once

#include <furi.h>

typedef struct Desktop Desktop;

#define RECORD_DESKTOP "desktop"

bool desktop_api_is_locked(Desktop* instance);

void desktop_api_unlock(Desktop* instance);

typedef struct {
    bool locked;
} DesktopStatus;

FuriPubSub* desktop_api_get_status_pubsub(Desktop* instance);
