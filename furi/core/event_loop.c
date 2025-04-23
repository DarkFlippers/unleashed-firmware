#include "event_loop_i.h"

#include "log.h"
#include "check.h"
#include "thread.h"

#include <FreeRTOS.h>
#include <task.h>

#define TAG "FuriEventLoop"

/*
 * Private functions
 */

static FuriEventLoopItem* furi_event_loop_item_alloc(
    FuriEventLoop* owner,
    const FuriEventLoopContract* contract,
    void* object,
    FuriEventLoopEvent event);

static void furi_event_loop_item_free(FuriEventLoopItem* instance);

static void furi_event_loop_item_free_later(FuriEventLoopItem* instance);

static void furi_event_loop_item_set_callback(
    FuriEventLoopItem* instance,
    FuriEventLoopEventCallback callback,
    void* callback_context);

static void furi_event_loop_item_notify(FuriEventLoopItem* instance);

static bool furi_event_loop_item_is_waiting(FuriEventLoopItem* instance);

static bool furi_event_loop_signal_callback(uint32_t signal, void* arg, void* context) {
    furi_assert(context);
    FuriEventLoop* instance = context;
    UNUSED(arg);

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance);
        return true;
    // Room for possible other standard signal handlers
    default:
        return false;
    }
}

/*
 * Main public API
 */

FuriEventLoop* furi_event_loop_alloc(void) {
    FuriEventLoop* instance = malloc(sizeof(FuriEventLoop));

    instance->thread_id = furi_thread_get_current_id();

    FuriEventLoopTree_init(instance->tree);
    WaitingList_init(instance->waiting_list);
    TimerList_init(instance->timer_list);
    TimerQueue_init(instance->timer_queue);
    PendingQueue_init(instance->pending_queue);

    // Clear notification state and value
    TaskHandle_t task = (TaskHandle_t)instance->thread_id;
    xTaskNotifyStateClearIndexed(task, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX);
    ulTaskNotifyValueClearIndexed(task, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, 0xFFFFFFFF);

    return instance;
}

void furi_event_loop_free(FuriEventLoop* instance) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(instance->state == FuriEventLoopStateStopped);

    furi_event_loop_process_timer_queue(instance);
    furi_check(TimerList_empty_p(instance->timer_list));
    furi_check(WaitingList_empty_p(instance->waiting_list));
    furi_check(!instance->are_thread_flags_subscribed);

    FuriEventLoopTree_clear(instance->tree);
    PendingQueue_clear(instance->pending_queue);

    uint32_t flags = 0;
    BaseType_t ret = xTaskNotifyWaitIndexed(
        FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, 0, FuriEventLoopFlagAll, &flags, 0);
    if(ret == pdTRUE) {
        FURI_LOG_D(TAG, "Some events were not processed: 0x%lx", flags);
    }

    free(instance);
}

static inline FuriEventLoopProcessStatus
    furi_event_loop_process_edge_event(FuriEventLoopItem* item) {
    FuriEventLoopProcessStatus status = FuriEventLoopProcessStatusComplete;
    item->callback(item->object, item->callback_context);

    return status;
}

static inline FuriEventLoopProcessStatus
    furi_event_loop_process_level_event(FuriEventLoopItem* item) {
    FuriEventLoopProcessStatus status = FuriEventLoopProcessStatusComplete;
    if(item->contract->get_level(item->object, item->event)) {
        item->callback(item->object, item->callback_context);

        if(item->contract->get_level(item->object, item->event)) {
            status = FuriEventLoopProcessStatusIncomplete;
        }
    }

    return status;
}

static inline FuriEventLoopProcessStatus
    furi_event_loop_process_event(FuriEventLoop* instance, FuriEventLoopItem* item) {
    FuriEventLoopProcessStatus status;

    if(item->event & FuriEventLoopEventFlagOnce) {
        furi_event_loop_unsubscribe(instance, item->object);
    }

    instance->current_item = item;

    if(item->event & FuriEventLoopEventFlagEdge) {
        status = furi_event_loop_process_edge_event(item);
    } else {
        status = furi_event_loop_process_level_event(item);
    }

    instance->current_item = NULL;

    if(item->owner == NULL) {
        status = FuriEventLoopProcessStatusFreeLater;
    }

    return status;
}

static inline FuriEventLoopItem* furi_event_loop_get_waiting_item(FuriEventLoop* instance) {
    FuriEventLoopItem* item = NULL;

    FURI_CRITICAL_ENTER();

    if(!WaitingList_empty_p(instance->waiting_list)) {
        item = WaitingList_pop_front(instance->waiting_list);
        WaitingList_init_field(item);
    }

    FURI_CRITICAL_EXIT();

    return item;
}

static inline void furi_event_loop_sync_flags(FuriEventLoop* instance) {
    FURI_CRITICAL_ENTER();

    if(!WaitingList_empty_p(instance->waiting_list)) {
        xTaskNotifyIndexed(
            (TaskHandle_t)instance->thread_id,
            FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
            FuriEventLoopFlagEvent,
            eSetBits);
    }

    FURI_CRITICAL_EXIT();
}

static void furi_event_loop_process_waiting_list(FuriEventLoop* instance) {
    FuriEventLoopItem* item = furi_event_loop_get_waiting_item(instance);
    if(!item) return;

    FuriEventLoopProcessStatus status = furi_event_loop_process_event(instance, item);

    if(status == FuriEventLoopProcessStatusComplete) {
        // Event processing complete, do nothing
    } else if(status == FuriEventLoopProcessStatusIncomplete) {
        // Event processing incomplete, put item back in waiting list
        furi_event_loop_item_notify(item);
    } else if(status == FuriEventLoopProcessStatusFreeLater) { //-V547
        // Unsubscribed from inside the callback, delete item
        furi_event_loop_item_free(item);
    } else {
        furi_crash();
    }

    furi_event_loop_sync_flags(instance);
}

static void furi_event_loop_process_pending_callbacks(FuriEventLoop* instance) {
    for(; !PendingQueue_empty_p(instance->pending_queue);
        PendingQueue_pop_back(NULL, instance->pending_queue)) {
        const FuriEventLoopPendingQueueItem* item = PendingQueue_back(instance->pending_queue);
        item->callback(item->context);
    }
}

static void furi_event_loop_restore_flags(FuriEventLoop* instance, uint32_t flags) {
    if(flags) {
        xTaskNotifyIndexed(
            (TaskHandle_t)instance->thread_id, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, flags, eSetBits);
    }
}

void furi_event_loop_run(FuriEventLoop* instance) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    FuriThread* thread = furi_thread_get_current();

    // Set the default signal callback if none was previously set
    if(furi_thread_get_signal_callback(thread) == NULL) {
        furi_thread_set_signal_callback(thread, furi_event_loop_signal_callback, instance);
    }

    furi_event_loop_init_tick(instance);

    instance->state = FuriEventLoopStateRunning;

    while(true) {
        const TickType_t ticks_to_sleep =
            MIN(furi_event_loop_get_timer_wait_time(instance),
                furi_event_loop_get_tick_wait_time(instance));

        uint32_t flags = 0;
        BaseType_t ret = xTaskNotifyWaitIndexed(
            FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, 0, FuriEventLoopFlagAll, &flags, ticks_to_sleep);

        if(ret == pdTRUE) {
            if(flags & FuriEventLoopFlagStop) {
                instance->state = FuriEventLoopStateStopped;
                break;

            } else if(flags & FuriEventLoopFlagEvent) {
                furi_event_loop_process_waiting_list(instance);
                furi_event_loop_restore_flags(instance, flags & ~FuriEventLoopFlagEvent);

            } else if(flags & FuriEventLoopFlagTimer) {
                furi_event_loop_process_timer_queue(instance);
                furi_event_loop_restore_flags(instance, flags & ~FuriEventLoopFlagTimer);

            } else if(flags & FuriEventLoopFlagPending) {
                furi_event_loop_process_pending_callbacks(instance);

            } else if(flags & FuriEventLoopFlagThreadFlag) {
                if(instance->are_thread_flags_subscribed)
                    instance->thread_flags_callback(instance->thread_flags_callback_context);

            } else {
                furi_crash();
            }

        } else if(!furi_event_loop_process_expired_timers(instance)) {
            furi_event_loop_process_tick(instance);
        }
    }

    // Disable the default signal callback
    if(furi_thread_get_signal_callback(thread) == furi_event_loop_signal_callback) {
        furi_thread_set_signal_callback(thread, NULL, NULL);
    }
}

static void furi_event_loop_notify(FuriEventLoop* instance, FuriEventLoopFlag flag) {
    if(FURI_IS_IRQ_MODE()) {
        BaseType_t yield = pdFALSE;

        (void)xTaskNotifyIndexedFromISR(
            (TaskHandle_t)instance->thread_id,
            FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
            flag,
            eSetBits,
            &yield);

        portYIELD_FROM_ISR(yield);

    } else {
        (void)xTaskNotifyIndexed(
            (TaskHandle_t)instance->thread_id, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, flag, eSetBits);
    }
}

void furi_event_loop_stop(FuriEventLoop* instance) {
    furi_check(instance);
    furi_event_loop_notify(instance, FuriEventLoopFlagStop);
}

/*
 * Public deferred function call API
 */

void furi_event_loop_pend_callback(
    FuriEventLoop* instance,
    FuriEventLoopPendingCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(callback);

    const FuriEventLoopPendingQueueItem item = {
        .callback = callback,
        .context = context,
    };

    PendingQueue_push_front(instance->pending_queue, item);

    furi_event_loop_notify(instance, FuriEventLoopFlagPending);
}

/*
 * Private generic susbscription API
 */

static void furi_event_loop_object_subscribe(
    FuriEventLoop* instance,
    FuriEventLoopObject* object,
    const FuriEventLoopContract* contract,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(object);
    furi_assert(contract);
    furi_check(callback);

    FURI_CRITICAL_ENTER();

    furi_check(FuriEventLoopTree_get(instance->tree, object) == NULL);

    // Allocate and setup item
    FuriEventLoopItem* item = furi_event_loop_item_alloc(instance, contract, object, event);
    furi_event_loop_item_set_callback(item, callback, context);

    FuriEventLoopTree_set_at(instance->tree, object, item);

    FuriEventLoopLink* link = item->contract->get_link(object);
    FuriEventLoopEvent event_noflags = item->event & FuriEventLoopEventMask;

    if(event_noflags == FuriEventLoopEventIn) {
        furi_check(link->item_in == NULL);
        link->item_in = item;
    } else if(event_noflags == FuriEventLoopEventOut) {
        furi_check(link->item_out == NULL);
        link->item_out = item;
    } else {
        furi_crash();
    }

    if(!(item->event & FuriEventLoopEventFlagEdge)) {
        if(item->contract->get_level(item->object, event_noflags)) {
            furi_event_loop_item_notify(item);
        }
    }

    FURI_CRITICAL_EXIT();
}

/**
 * Public specialized subscription API
 */

void furi_event_loop_subscribe_event_flag(
    FuriEventLoop* instance,
    FuriEventFlag* event_flag,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    extern const FuriEventLoopContract furi_event_flag_event_loop_contract;
    furi_event_loop_object_subscribe(
        instance, event_flag, &furi_event_flag_event_loop_contract, event, callback, context);
}

void furi_event_loop_subscribe_message_queue(
    FuriEventLoop* instance,
    FuriMessageQueue* message_queue,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    extern const FuriEventLoopContract furi_message_queue_event_loop_contract;

    furi_event_loop_object_subscribe(
        instance, message_queue, &furi_message_queue_event_loop_contract, event, callback, context);
}

void furi_event_loop_subscribe_stream_buffer(
    FuriEventLoop* instance,
    FuriStreamBuffer* stream_buffer,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    extern const FuriEventLoopContract furi_stream_buffer_event_loop_contract;

    furi_event_loop_object_subscribe(
        instance, stream_buffer, &furi_stream_buffer_event_loop_contract, event, callback, context);
}

void furi_event_loop_subscribe_semaphore(
    FuriEventLoop* instance,
    FuriSemaphore* semaphore,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    extern const FuriEventLoopContract furi_semaphore_event_loop_contract;

    furi_event_loop_object_subscribe(
        instance, semaphore, &furi_semaphore_event_loop_contract, event, callback, context);
}

void furi_event_loop_subscribe_mutex(
    FuriEventLoop* instance,
    FuriMutex* mutex,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    extern const FuriEventLoopContract furi_mutex_event_loop_contract;

    furi_event_loop_object_subscribe(
        instance, mutex, &furi_mutex_event_loop_contract, event, callback, context);
}

void furi_event_loop_subscribe_thread_flags(
    FuriEventLoop* instance,
    FuriEventLoopThreadFlagsCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(callback);
    furi_check(!instance->are_thread_flags_subscribed);
    instance->are_thread_flags_subscribed = true;
    instance->thread_flags_callback = callback;
    instance->thread_flags_callback_context = context;
}

void furi_event_loop_unsubscribe_thread_flags(FuriEventLoop* instance) {
    furi_check(instance);
    furi_check(instance->are_thread_flags_subscribed);
    instance->are_thread_flags_subscribed = false;
}

/**
 * Public generic unsubscription API
 */

void furi_event_loop_unsubscribe(FuriEventLoop* instance, FuriEventLoopObject* object) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());

    FURI_CRITICAL_ENTER();

    FuriEventLoopItem* item = NULL;
    furi_check(FuriEventLoopTree_pop_at(&item, instance->tree, object));

    furi_check(item);
    furi_check(item->owner == instance);

    FuriEventLoopLink* link = item->contract->get_link(object);
    FuriEventLoopEvent event_noflags = item->event & FuriEventLoopEventMask;

    if(event_noflags == FuriEventLoopEventIn) {
        furi_check(link->item_in == item);
        link->item_in = NULL;
    } else if(event_noflags == FuriEventLoopEventOut) {
        furi_check(link->item_out == item);
        link->item_out = NULL;
    } else {
        furi_crash();
    }

    if(furi_event_loop_item_is_waiting(item)) {
        WaitingList_unlink(item);
    }

    if(instance->current_item == item) {
        furi_event_loop_item_free_later(item);
    } else {
        furi_event_loop_item_free(item);
    }

    FURI_CRITICAL_EXIT();
}

bool furi_event_loop_is_subscribed(FuriEventLoop* instance, FuriEventLoopObject* object) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    FURI_CRITICAL_ENTER();

    FuriEventLoopItem* const* item = FuriEventLoopTree_cget(instance->tree, object);
    bool result = !!item;

    FURI_CRITICAL_EXIT();
    return result;
}

/* 
 * Private Event Loop Item functions
 */

static FuriEventLoopItem* furi_event_loop_item_alloc(
    FuriEventLoop* owner,
    const FuriEventLoopContract* contract,
    void* object,
    FuriEventLoopEvent event) {
    furi_assert(owner);
    furi_assert(object);

    FuriEventLoopItem* instance = malloc(sizeof(FuriEventLoopItem));

    instance->owner = owner;
    instance->contract = contract;
    instance->object = object;
    instance->event = event;

    WaitingList_init_field(instance);

    return instance;
}

static void furi_event_loop_item_free(FuriEventLoopItem* instance) {
    furi_assert(instance);
    furi_assert(!furi_event_loop_item_is_waiting(instance));
    free(instance);
}

static void furi_event_loop_item_free_later(FuriEventLoopItem* instance) {
    furi_assert(instance);
    furi_assert(!furi_event_loop_item_is_waiting(instance));
    instance->owner = NULL;
}

static void furi_event_loop_item_set_callback(
    FuriEventLoopItem* instance,
    FuriEventLoopEventCallback callback,
    void* callback_context) {
    furi_assert(instance);
    furi_assert(!instance->callback);

    instance->callback = callback;
    instance->callback_context = callback_context;
}

static void furi_event_loop_item_notify(FuriEventLoopItem* instance) {
    furi_assert(instance);

    FURI_CRITICAL_ENTER();

    FuriEventLoop* owner = instance->owner;
    furi_assert(owner);

    if(!furi_event_loop_item_is_waiting(instance)) {
        WaitingList_push_back(owner->waiting_list, instance);
    }

    FURI_CRITICAL_EXIT();

    furi_event_loop_notify(owner, FuriEventLoopFlagEvent);
}

static bool furi_event_loop_item_is_waiting(FuriEventLoopItem* instance) {
    return instance->WaitingList.prev || instance->WaitingList.next;
}

void furi_event_loop_thread_flag_callback(FuriThreadId thread_id) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    BaseType_t yield;

    if(FURI_IS_IRQ_MODE()) {
        yield = pdFALSE;
        (void)xTaskNotifyIndexedFromISR(
            hTask,
            FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
            FuriEventLoopFlagThreadFlag,
            eSetBits,
            &yield);
        portYIELD_FROM_ISR(yield);
    } else {
        (void)xTaskNotifyIndexed(
            hTask, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, FuriEventLoopFlagThreadFlag, eSetBits);
    }
}

/*
 * Internal event loop link API, used by supported primitives
 */

void furi_event_loop_link_notify(FuriEventLoopLink* instance, FuriEventLoopEvent event) {
    furi_assert(instance);

    FURI_CRITICAL_ENTER();

    if(event & FuriEventLoopEventIn) {
        if(instance->item_in) furi_event_loop_item_notify(instance->item_in);
    } else if(event & FuriEventLoopEventOut) {
        if(instance->item_out) furi_event_loop_item_notify(instance->item_out);
    } else {
        furi_crash();
    }

    FURI_CRITICAL_EXIT();
}
