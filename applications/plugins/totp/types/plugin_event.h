#pragma once

#include <inttypes.h>
#include <input/input.h>
#include "event_type.h"

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;
