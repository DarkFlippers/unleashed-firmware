#include "event_loop_i.h"

#include <FreeRTOS.h>
#include <task.h>

#include <furi.h>

/**
 * Private functions
 */

static inline uint32_t furi_event_loop_tick_get_elapsed_time(const FuriEventLoop* instance) {
    return xTaskGetTickCount() - instance->tick.prev_time;
}

static inline uint32_t furi_event_loop_tick_get_remaining_time(const FuriEventLoop* instance) {
    const uint32_t elapsed_time = furi_event_loop_tick_get_elapsed_time(instance);
    return elapsed_time < instance->tick.interval ? instance->tick.interval - elapsed_time : 0;
}

static inline bool furi_event_loop_tick_is_expired(const FuriEventLoop* instance) {
    return furi_event_loop_tick_get_elapsed_time(instance) >= instance->tick.interval;
}

/*
 * Private tick API
 */

void furi_event_loop_init_tick(FuriEventLoop* instance) {
    if(instance->tick.callback) {
        instance->tick.prev_time = xTaskGetTickCount();
    }
}

void furi_event_loop_process_tick(FuriEventLoop* instance) {
    if(instance->tick.callback && furi_event_loop_tick_is_expired(instance)) {
        instance->tick.prev_time += instance->tick.interval;
        instance->tick.callback(instance->tick.callback_context);
    }
}

uint32_t furi_event_loop_get_tick_wait_time(const FuriEventLoop* instance) {
    uint32_t wait_time = FuriWaitForever;

    if(instance->tick.callback) {
        wait_time = furi_event_loop_tick_get_remaining_time(instance);
    }

    return wait_time;
}

/*
 * Public tick API
 */

void furi_event_loop_tick_set(
    FuriEventLoop* instance,
    uint32_t interval,
    FuriEventLoopTickCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(callback ? interval > 0 : true);

    instance->tick.callback = callback;
    instance->tick.callback_context = context;
    instance->tick.interval = interval;
    instance->tick.prev_time = xTaskGetTickCount();
}
