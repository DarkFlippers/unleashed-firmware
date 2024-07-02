#pragma once

#include "event_loop_timer.h"

#include <m-i-list.h>

typedef enum {
    FuriEventLoopTimerRequestNone,
    FuriEventLoopTimerRequestStart,
    FuriEventLoopTimerRequestStop,
    FuriEventLoopTimerRequestFree,
} FuriEventLoopTimerRequest;

struct FuriEventLoopTimer {
    FuriEventLoop* owner;

    FuriEventLoopTimerCallback callback;
    void* context;

    uint32_t interval;
    uint32_t start_time;
    uint32_t next_interval;

    // Interface for the active timer list
    ILIST_INTERFACE(TimerList, FuriEventLoopTimer);

    // Interface for the timer request queue
    ILIST_INTERFACE(TimerQueue, FuriEventLoopTimer);

    FuriEventLoopTimerRequest request;

    bool active;
    bool periodic;
};

ILIST_DEF(TimerList, FuriEventLoopTimer, M_POD_OPLIST)
ILIST_DEF(TimerQueue, FuriEventLoopTimer, M_POD_OPLIST)

uint32_t furi_event_loop_get_timer_wait_time(const FuriEventLoop* instance);

void furi_event_loop_process_timer_queue(FuriEventLoop* instance);

bool furi_event_loop_process_expired_timers(FuriEventLoop* instance);
