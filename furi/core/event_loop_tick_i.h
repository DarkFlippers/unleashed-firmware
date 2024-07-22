#pragma once

#include "event_loop.h"

typedef struct {
    uint32_t interval;
    uint32_t prev_time;
    FuriEventLoopTickCallback callback;
    void* callback_context;
} FuriEventLoopTick;

void furi_event_loop_init_tick(FuriEventLoop* instance);

void furi_event_loop_process_tick(FuriEventLoop* instance);

uint32_t furi_event_loop_get_tick_wait_time(const FuriEventLoop* instance);
