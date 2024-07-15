#include "dolphin_i.h"

#include <furi_hal.h>

#define TAG "Dolphin"

#define DOLPHIN_LOCK_EVENT_FLAG (0x1)
#define EVENT_QUEUE_SIZE        (8)

#define SECONDS_IN_TICKS(x)    ((x) * 1000UL)
#define MINUTES_IN_TICKS(x)    (SECONDS_IN_TICKS(x) * 60UL)
#define HOURS_IN_TICKS(x)      (MINUTES_IN_TICKS(x) * 60UL)
#define DATE_IN_TICKS(h, m, s) (HOURS_IN_TICKS(h) + MINUTES_IN_TICKS(m) + SECONDS_IN_TICKS(s))

#define FLUSH_TIMEOUT_TICKS (SECONDS_IN_TICKS(30UL))

#ifndef DOLPHIN_DEBUG
#define BUTTHURT_INCREASE_PERIOD_TICKS   (HOURS_IN_TICKS(48UL))
#define CLEAR_LIMITS_PERIOD_TICKS        (HOURS_IN_TICKS(24UL))
#define CLEAR_LIMITS_UPDATE_PERIOD_TICKS (HOURS_IN_TICKS(1UL))
#else
#define BUTTHURT_INCREASE_PERIOD_TICKS   (SECONDS_IN_TICKS(30UL))
#define CLEAR_LIMITS_PERIOD_TICKS        (MINUTES_IN_TICKS(1))
#define CLEAR_LIMITS_UPDATE_PERIOD_TICKS (SECONDS_IN_TICKS(5UL))
#endif

#define CLEAR_LIMITS_UPDATE_THRESHOLD_TICKS (MINUTES_IN_TICKS(5UL))

#define CLEAR_LIMITS_TIME_HOURS (5UL)
#define CLEAR_LIMITS_TIME_TICKS (HOURS_IN_TICKS(CLEAR_LIMITS_TIME_HOURS))

static void dolphin_event_send_async(Dolphin* dolphin, DolphinEvent* event);
static void dolphin_event_send_wait(Dolphin* dolphin, DolphinEvent* event);

// Public API

void dolphin_deed(DolphinDeed deed) {
    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);

    DolphinEvent event;
    event.type = DolphinEventTypeDeed;
    event.deed = deed;

    dolphin_event_send_async(dolphin, &event);

    furi_record_close(RECORD_DOLPHIN);
}

DolphinStats dolphin_stats(Dolphin* dolphin) {
    furi_check(dolphin);

    DolphinStats stats;
    DolphinEvent event;

    event.type = DolphinEventTypeStats;
    event.stats = &stats;

    dolphin_event_send_wait(dolphin, &event);

    return stats;
}

void dolphin_flush(Dolphin* dolphin) {
    furi_check(dolphin);

    DolphinEvent event;
    event.type = DolphinEventTypeFlush;

    dolphin_event_send_wait(dolphin, &event);
}

void dolphin_upgrade_level(Dolphin* dolphin) {
    furi_check(dolphin);

    DolphinEvent event;
    event.type = DolphinEventTypeLevel;

    dolphin_event_send_async(dolphin, &event);
}

FuriPubSub* dolphin_get_pubsub(Dolphin* dolphin) {
    furi_check(dolphin);
    return dolphin->pubsub;
}

// Private functions

static void dolphin_butthurt_timer_callback(void* context) {
    Dolphin* dolphin = context;
    furi_assert(dolphin);

    FURI_LOG_I(TAG, "Increase butthurt");
    dolphin_state_butthurted(dolphin->state);
    dolphin_state_save(dolphin->state);
}

static void dolphin_flush_timer_callback(void* context) {
    Dolphin* dolphin = context;
    furi_assert(dolphin);

    FURI_LOG_I(TAG, "Flush stats");
    dolphin_state_save(dolphin->state);
}

static void dolphin_clear_limits_timer_callback(void* context) {
    Dolphin* dolphin = context;
    furi_assert(dolphin);

    FURI_LOG_I(TAG, "Clear limits");
    dolphin_state_clear_limits(dolphin->state);
    dolphin_state_save(dolphin->state);
}

static Dolphin* dolphin_alloc(void) {
    Dolphin* dolphin = malloc(sizeof(Dolphin));

    dolphin->state = dolphin_state_alloc();
    dolphin->pubsub = furi_pubsub_alloc();
    dolphin->event_queue = furi_message_queue_alloc(EVENT_QUEUE_SIZE, sizeof(DolphinEvent));
    dolphin->event_loop = furi_event_loop_alloc();

    dolphin->butthurt_timer = furi_event_loop_timer_alloc(
        dolphin->event_loop,
        dolphin_butthurt_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        dolphin);

    dolphin->flush_timer = furi_event_loop_timer_alloc(
        dolphin->event_loop, dolphin_flush_timer_callback, FuriEventLoopTimerTypeOnce, dolphin);

    dolphin->clear_limits_timer = furi_event_loop_timer_alloc(
        dolphin->event_loop,
        dolphin_clear_limits_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        dolphin);

    return dolphin;
}

static void dolphin_event_send_async(Dolphin* dolphin, DolphinEvent* event) {
    furi_assert(dolphin);
    furi_assert(event);
    event->flag = NULL;
    furi_check(
        furi_message_queue_put(dolphin->event_queue, event, FuriWaitForever) == FuriStatusOk);
}

static void dolphin_event_send_wait(Dolphin* dolphin, DolphinEvent* event) {
    furi_assert(dolphin);
    furi_assert(event);

    event->flag = furi_event_flag_alloc();
    furi_check(
        furi_message_queue_put(dolphin->event_queue, event, FuriWaitForever) == FuriStatusOk);
    furi_check(
        furi_event_flag_wait(
            event->flag, DOLPHIN_LOCK_EVENT_FLAG, FuriFlagWaitAny, FuriWaitForever) ==
        DOLPHIN_LOCK_EVENT_FLAG);
    furi_event_flag_free(event->flag);
}

static void dolphin_event_release(DolphinEvent* event) {
    if(event->flag) {
        furi_event_flag_set(event->flag, DOLPHIN_LOCK_EVENT_FLAG);
    }
}

static void dolphin_update_clear_limits_timer_period(void* context) {
    furi_assert(context);
    Dolphin* dolphin = context;

    uint32_t time_to_clear_limits =
        furi_event_loop_timer_get_remaining_time(dolphin->clear_limits_timer);

    if(time_to_clear_limits > CLEAR_LIMITS_UPDATE_THRESHOLD_TICKS) {
        DateTime date;
        furi_hal_rtc_get_datetime(&date);

        const uint32_t now_time_ticks = DATE_IN_TICKS(date.hour, date.minute, date.second);

        if(date.hour < CLEAR_LIMITS_TIME_HOURS) {
            time_to_clear_limits = CLEAR_LIMITS_TIME_TICKS - now_time_ticks;
        } else {
            time_to_clear_limits =
                CLEAR_LIMITS_PERIOD_TICKS + CLEAR_LIMITS_TIME_TICKS - now_time_ticks;
        }

        furi_event_loop_timer_start(dolphin->clear_limits_timer, time_to_clear_limits);
    }

    FURI_LOG_D(TAG, "Daily limits reset in %lu ms", time_to_clear_limits);
}

static bool dolphin_process_event(FuriMessageQueue* queue, void* context) {
    UNUSED(queue);

    Dolphin* dolphin = context;
    DolphinEvent event;

    FuriStatus status = furi_message_queue_get(dolphin->event_queue, &event, 0);
    furi_check(status == FuriStatusOk);

    if(event.type == DolphinEventTypeDeed) {
        dolphin_state_on_deed(dolphin->state, event.deed);

        DolphinPubsubEvent event = DolphinPubsubEventUpdate;
        furi_pubsub_publish(dolphin->pubsub, &event);
        furi_event_loop_timer_start(dolphin->butthurt_timer, BUTTHURT_INCREASE_PERIOD_TICKS);
        furi_event_loop_timer_start(dolphin->flush_timer, FLUSH_TIMEOUT_TICKS);

    } else if(event.type == DolphinEventTypeStats) {
        event.stats->icounter = dolphin->state->data.icounter;
        event.stats->butthurt = dolphin->state->data.butthurt;
        event.stats->timestamp = dolphin->state->data.timestamp;
        event.stats->level = dolphin_get_level(dolphin->state->data.icounter);
        event.stats->level_up_is_pending =
            !dolphin_state_xp_to_levelup(dolphin->state->data.icounter);

    } else if(event.type == DolphinEventTypeFlush) {
        furi_event_loop_timer_start(dolphin->flush_timer, FLUSH_TIMEOUT_TICKS);

    } else if(event.type == DolphinEventTypeLevel) {
        dolphin_state_increase_level(dolphin->state);
        furi_event_loop_timer_start(dolphin->flush_timer, FLUSH_TIMEOUT_TICKS);

    } else {
        furi_crash();
    }

    dolphin_event_release(&event);

    return true;
}

// Application thread

int32_t dolphin_srv(void* p) {
    UNUSED(p);

    if(furi_hal_rtc_get_boot_mode() != FuriHalRtcBootModeNormal) {
        FURI_LOG_W(TAG, "Skipping start in special boot mode");

        furi_thread_suspend(furi_thread_get_current_id());
        return 0;
    }

    Dolphin* dolphin = dolphin_alloc();
    furi_record_create(RECORD_DOLPHIN, dolphin);

    dolphin_state_load(dolphin->state);

    furi_event_loop_message_queue_subscribe(
        dolphin->event_loop,
        dolphin->event_queue,
        FuriEventLoopEventIn,
        dolphin_process_event,
        dolphin);

    furi_event_loop_timer_start(dolphin->butthurt_timer, BUTTHURT_INCREASE_PERIOD_TICKS);
    furi_event_loop_timer_start(dolphin->clear_limits_timer, CLEAR_LIMITS_PERIOD_TICKS);

    furi_event_loop_tick_set(
        dolphin->event_loop,
        CLEAR_LIMITS_UPDATE_PERIOD_TICKS,
        dolphin_update_clear_limits_timer_period,
        dolphin);

    furi_event_loop_pend_callback(
        dolphin->event_loop, dolphin_update_clear_limits_timer_period, dolphin);

    furi_event_loop_run(dolphin->event_loop);

    return 0;
}
