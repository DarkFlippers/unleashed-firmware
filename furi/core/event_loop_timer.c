#include "event_loop_i.h"

#include <FreeRTOS.h>
#include <task.h>

#include <furi.h>

/*
 * Private functions
 */

static inline uint32_t furi_event_loop_timer_get_elapsed_time(const FuriEventLoopTimer* timer) {
    return xTaskGetTickCount() - timer->start_time;
}

static inline uint32_t
    furi_event_loop_timer_get_remaining_time_private(const FuriEventLoopTimer* timer) {
    const uint32_t elapsed_time = furi_event_loop_timer_get_elapsed_time(timer);
    return elapsed_time < timer->interval ? timer->interval - elapsed_time : 0;
}

static inline bool furi_event_loop_timer_is_expired(const FuriEventLoopTimer* timer) {
    return furi_event_loop_timer_get_elapsed_time(timer) >= timer->interval;
}

static void furi_event_loop_schedule_timer(FuriEventLoop* instance, FuriEventLoopTimer* timer) {
    FuriEventLoopTimer* timer_pos = NULL;

    FURI_CRITICAL_ENTER();

    const uint32_t remaining_time = furi_event_loop_timer_get_remaining_time_private(timer);

    TimerList_it_t it;
    for(TimerList_it_last(it, instance->timer_list); !TimerList_end_p(it);
        TimerList_previous(it)) {
        FuriEventLoopTimer* tmp = TimerList_ref(it);
        if(remaining_time >= furi_event_loop_timer_get_remaining_time_private(tmp)) {
            timer_pos = tmp;
            break;
        }
    }

    FURI_CRITICAL_EXIT();

    if(timer_pos) {
        TimerList_push_after(timer_pos, timer);
    } else {
        TimerList_push_front(instance->timer_list, timer);
    }
    // At this point, TimerList_front() points to the first timer to expire
}

static void furi_event_loop_timer_enqueue_request(
    FuriEventLoopTimer* timer,
    FuriEventLoopTimerRequest request) {
    if(timer->request != FuriEventLoopTimerRequestNone) {
        // You cannot change your mind after calling furi_event_loop_timer_free()
        furi_check(timer->request != FuriEventLoopTimerRequestFree);
        TimerQueue_unlink(timer);
    }

    timer->request = request;

    FuriEventLoop* instance = timer->owner;
    TimerQueue_push_back(instance->timer_queue, timer);

    xTaskNotifyIndexed(
        (TaskHandle_t)instance->thread_id,
        FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
        FuriEventLoopFlagTimer,
        eSetBits);
}

/*
 * Private API
 */

uint32_t furi_event_loop_get_timer_wait_time(const FuriEventLoop* instance) {
    uint32_t wait_time = FuriWaitForever;

    if(!TimerList_empty_p(instance->timer_list)) {
        FuriEventLoopTimer* timer = TimerList_front(instance->timer_list);
        wait_time = furi_event_loop_timer_get_remaining_time_private(timer);
    }

    return wait_time;
}

void furi_event_loop_process_timer_queue(FuriEventLoop* instance) {
    while(!TimerQueue_empty_p(instance->timer_queue)) {
        FuriEventLoopTimer* timer = TimerQueue_pop_front(instance->timer_queue);

        if(timer->active) {
            TimerList_unlink(timer);
        }

        if(timer->request == FuriEventLoopTimerRequestStart) {
            timer->active = true;
            timer->interval = timer->next_interval;
            timer->start_time = xTaskGetTickCount();
            timer->request = FuriEventLoopTimerRequestNone;

            furi_event_loop_schedule_timer(instance, timer);

        } else if(timer->request == FuriEventLoopTimerRequestStop) {
            timer->active = false;
            timer->request = FuriEventLoopTimerRequestNone;

        } else if(timer->request == FuriEventLoopTimerRequestFree) {
            free(timer);

        } else {
            furi_crash();
        }
    }
}

bool furi_event_loop_process_expired_timers(FuriEventLoop* instance) {
    if(TimerList_empty_p(instance->timer_list)) {
        return false;
    }
    // The front() element contains the earliest-expiring timer
    FuriEventLoopTimer* timer = TimerList_front(instance->timer_list);

    if(!furi_event_loop_timer_is_expired(timer)) {
        return false;
    }

    TimerList_unlink(timer);

    if(timer->periodic) {
        const uint32_t num_events =
            furi_event_loop_timer_get_elapsed_time(timer) / timer->interval;

        timer->start_time += timer->interval * num_events;
        furi_event_loop_schedule_timer(instance, timer);

    } else {
        timer->active = false;
    }

    timer->callback(timer->context);
    return true;
}

/*
 * Public timer API
 */

FuriEventLoopTimer* furi_event_loop_timer_alloc(
    FuriEventLoop* instance,
    FuriEventLoopTimerCallback callback,
    FuriEventLoopTimerType type,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(callback);
    furi_check(type <= FuriEventLoopTimerTypePeriodic);

    FuriEventLoopTimer* timer = malloc(sizeof(FuriEventLoopTimer));

    timer->owner = instance;
    timer->callback = callback;
    timer->context = context;
    timer->periodic = (type == FuriEventLoopTimerTypePeriodic);

    TimerList_init_field(timer);
    TimerQueue_init_field(timer);

    return timer;
}

void furi_event_loop_timer_free(FuriEventLoopTimer* timer) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestFree);
}

void furi_event_loop_timer_start(FuriEventLoopTimer* timer, uint32_t interval) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    timer->next_interval = interval;

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestStart);
}

void furi_event_loop_timer_restart(FuriEventLoopTimer* timer) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    timer->next_interval = timer->interval;

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestStart);
}

void furi_event_loop_timer_stop(FuriEventLoopTimer* timer) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestStop);
}

uint32_t furi_event_loop_timer_get_remaining_time(const FuriEventLoopTimer* timer) {
    furi_check(timer);
    return furi_event_loop_timer_get_remaining_time_private(timer);
}

uint32_t furi_event_loop_timer_get_interval(const FuriEventLoopTimer* timer) {
    furi_check(timer);
    return timer->interval;
}

bool furi_event_loop_timer_is_running(const FuriEventLoopTimer* timer) {
    furi_check(timer);
    return timer->active;
}
