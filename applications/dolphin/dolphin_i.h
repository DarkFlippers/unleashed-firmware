#pragma once

#include "furi/pubsub.h"
#include <furi.h>
#include <furi_hal.h>

#include "dolphin.h"
#include "helpers/dolphin_state.h"

typedef enum {
    DolphinEventTypeDeed,
    DolphinEventTypeStats,
    DolphinEventTypeFlush,
    DolphinEventTypeAnimationStartNewIdle,
    DolphinEventTypeAnimationCheckBlocking,
    DolphinEventTypeAnimationInteract,
} DolphinEventType;

typedef struct {
    DolphinEventType type;
    osEventFlagsId_t flag;
    union {
        DolphinDeed deed;
        DolphinStats* stats;
    };
} DolphinEvent;

struct Dolphin {
    // State
    DolphinState* state;
    // Queue
    osMessageQueueId_t event_queue;
    FuriPubSub* pubsub;
};

Dolphin* dolphin_alloc();

void dolphin_free(Dolphin* dolphin);

void dolphin_event_send_async(Dolphin* dolphin, DolphinEvent* event);

void dolphin_event_send_wait(Dolphin* dolphin, DolphinEvent* event);

void dolphin_event_release(Dolphin* dolphin, DolphinEvent* event);
