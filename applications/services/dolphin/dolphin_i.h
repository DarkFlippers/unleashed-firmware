#pragma once

#include <furi.h>

#include <core/pubsub.h>

#include "dolphin.h"
#include "helpers/dolphin_state.h"

typedef enum {
    DolphinEventTypeDeed,
    DolphinEventTypeStats,
    DolphinEventTypeFlush,
    DolphinEventTypeLevel,
} DolphinEventType;

typedef struct {
    DolphinEventType type;
    FuriEventFlag* flag;
    union {
        DolphinDeed deed;
        DolphinStats* stats;
    };
} DolphinEvent;

struct Dolphin {
    DolphinState* state;
    FuriPubSub* pubsub;
    FuriMessageQueue* event_queue;
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* butthurt_timer;
    FuriEventLoopTimer* flush_timer;
    FuriEventLoopTimer* clear_limits_timer;
};
