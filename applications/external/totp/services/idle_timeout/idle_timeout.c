#include "idle_timeout.h"
#include <stdlib.h>
#include <furi/core/timer.h>

#define IDLE_TIMER_CHECK_PERIODICITY_SEC (1)
#define SEC_TO_TICKS(sec) ((sec)*1000)

struct IdleTimeoutContext {
    FuriTimer* timer;
    bool activity_reported;
    void* on_idle_callback_context;
    IDLE_TIMEOUT_CALLBACK on_idle_callback;
    uint16_t timeout_sec;
    uint16_t idle_period_sec;
    bool idle_handled;
};

static void idle_timer_callback(void* context) {
    IdleTimeoutContext* instance = context;
    if(instance->activity_reported) {
        instance->idle_period_sec = 0;
        instance->idle_handled = false;
        instance->activity_reported = false;
    } else if(!instance->idle_handled) {
        if(instance->idle_period_sec >= instance->timeout_sec) {
            instance->idle_handled =
                instance->on_idle_callback(instance->on_idle_callback_context);
        } else {
            instance->idle_period_sec += IDLE_TIMER_CHECK_PERIODICITY_SEC;
        }
    }
}

IdleTimeoutContext* idle_timeout_alloc(
    uint16_t timeout_sec,
    IDLE_TIMEOUT_CALLBACK on_idle_callback,
    void* on_idle_callback_context) {
    IdleTimeoutContext* instance = malloc(sizeof(IdleTimeoutContext));
    if(instance == NULL) return NULL;

    instance->timer = furi_timer_alloc(&idle_timer_callback, FuriTimerTypePeriodic, instance);
    if(instance->timer == NULL) return NULL;

    instance->timeout_sec = timeout_sec;
    instance->on_idle_callback = on_idle_callback;
    instance->on_idle_callback_context = on_idle_callback_context;
    return instance;
}

void idle_timeout_start(IdleTimeoutContext* context) {
    furi_timer_start(context->timer, SEC_TO_TICKS(IDLE_TIMER_CHECK_PERIODICITY_SEC));
}

void idle_timeout_stop(IdleTimeoutContext* context) {
    furi_timer_stop(context->timer);
}

void idle_timeout_report_activity(IdleTimeoutContext* context) {
    context->activity_reported = true;
}

void idle_timeout_free(IdleTimeoutContext* context) {
    furi_timer_stop(context->timer);
    furi_timer_free(context->timer);
    free(context);
}