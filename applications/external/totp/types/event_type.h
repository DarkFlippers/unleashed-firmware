#pragma once
#include <inttypes.h>

typedef uint8_t EventType;

enum EventTypes { EventTypeTick, EventTypeKey, EventForceCloseApp, EventForceRedraw };
