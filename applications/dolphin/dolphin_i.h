#pragma once

#include <furi.h>
#include <furi-hal.h>

#include "dolphin.h"
#include "helpers/dolphin_state.h"

typedef enum {
    DolphinEventTypeDeed,
    DolphinEventTypeSave,
    DolphinEventTypeTick,
} DolphinEventType;

typedef struct {
    DolphinEventType type;
    union {
        DolphinDeed deed;
    };
} DolphinEvent;

struct Dolphin {
    // State
    DolphinState* state;
    // Queue
    osMessageQueueId_t event_queue;
};

Dolphin* dolphin_alloc();

void dolphin_free(Dolphin* dolphin);
