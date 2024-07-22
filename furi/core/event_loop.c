#include "event_loop_i.h"
#include "message_queue_i.h"

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

static void furi_event_loop_item_set_callback(
    FuriEventLoopItem* instance,
    FuriEventLoopMessageQueueCallback callback,
    void* callback_context);

static void furi_event_loop_item_notify(FuriEventLoopItem* instance);

static void furi_event_loop_process_pending_callbacks(FuriEventLoop* instance) {
    for(; !PendingQueue_empty_p(instance->pending_queue);
        PendingQueue_pop_back(NULL, instance->pending_queue)) {
        const FuriEventLoopPendingQueueItem* item = PendingQueue_back(instance->pending_queue);
        item->callback(item->context);
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
    xTaskNotifyStateClearIndexed(instance->thread_id, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX);
    ulTaskNotifyValueClearIndexed(
        instance->thread_id, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, 0xFFFFFFFF);

    return instance;
}

void furi_event_loop_free(FuriEventLoop* instance) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(instance->state == FuriEventLoopStateStopped);

    furi_event_loop_process_timer_queue(instance);
    furi_check(TimerList_empty_p(instance->timer_list));

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

static FuriEventLoopProcessStatus
    furi_event_loop_poll_process_event(FuriEventLoop* instance, FuriEventLoopItem* item) {
    UNUSED(instance);

    if(!item->contract->get_level(item->object, item->event)) {
        return FuriEventLoopProcessStatusComplete;
    }

    if(item->callback(item->object, item->callback_context)) {
        return FuriEventLoopProcessStatusIncomplete;
    } else {
        return FuriEventLoopProcessStatusAgain;
    }
}

static void furi_event_loop_restore_flags(FuriEventLoop* instance, uint32_t flags) {
    if(flags) {
        xTaskNotifyIndexed(
            instance->thread_id, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, flags, eSetBits);
    }
}

void furi_event_loop_run(FuriEventLoop* instance) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());

    furi_event_loop_init_tick(instance);

    furi_thread_set_signal_callback(
        instance->thread_id, furi_event_loop_signal_callback, instance);

    while(true) {
        instance->state = FuriEventLoopStateIdle;

        const TickType_t ticks_to_sleep =
            MIN(furi_event_loop_get_timer_wait_time(instance),
                furi_event_loop_get_tick_wait_time(instance));

        uint32_t flags = 0;
        BaseType_t ret = xTaskNotifyWaitIndexed(
            FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, 0, FuriEventLoopFlagAll, &flags, ticks_to_sleep);

        instance->state = FuriEventLoopStateProcessing;

        if(ret == pdTRUE) {
            if(flags & FuriEventLoopFlagStop) {
                instance->state = FuriEventLoopStateStopped;
                break;

            } else if(flags & FuriEventLoopFlagEvent) {
                FuriEventLoopItem* item = NULL;
                FURI_CRITICAL_ENTER();

                if(!WaitingList_empty_p(instance->waiting_list)) {
                    item = WaitingList_pop_front(instance->waiting_list);
                    WaitingList_init_field(item);
                }

                FURI_CRITICAL_EXIT();

                if(item) {
                    while(true) {
                        FuriEventLoopProcessStatus ret =
                            furi_event_loop_poll_process_event(instance, item);
                        if(ret == FuriEventLoopProcessStatusComplete) {
                            // Event processing complete, break from loop
                            break;
                        } else if(ret == FuriEventLoopProcessStatusIncomplete) {
                            // Event processing incomplete more processing needed
                        } else if(ret == FuriEventLoopProcessStatusAgain) { //-V547
                            furi_event_loop_item_notify(item);
                            break;
                        } else {
                            furi_crash();
                        }
                    }
                }

                furi_event_loop_restore_flags(instance, flags & ~FuriEventLoopFlagEvent);

            } else if(flags & FuriEventLoopFlagTimer) {
                furi_event_loop_process_timer_queue(instance);
                furi_event_loop_restore_flags(instance, flags & ~FuriEventLoopFlagTimer);

            } else if(flags & FuriEventLoopFlagPending) {
                furi_event_loop_process_pending_callbacks(instance);

            } else {
                furi_crash();
            }

        } else if(!furi_event_loop_process_expired_timers(instance)) {
            furi_event_loop_process_tick(instance);
        }
    }

    furi_thread_set_signal_callback(instance->thread_id, NULL, NULL);
}

void furi_event_loop_stop(FuriEventLoop* instance) {
    furi_check(instance);

    xTaskNotifyIndexed(
        instance->thread_id, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, FuriEventLoopFlagStop, eSetBits);
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

    xTaskNotifyIndexed(
        instance->thread_id, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, FuriEventLoopFlagPending, eSetBits);
}

/*
 * Message queue API
 */

void furi_event_loop_message_queue_subscribe(
    FuriEventLoop* instance,
    FuriMessageQueue* message_queue,
    FuriEventLoopEvent event,
    FuriEventLoopMessageQueueCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(instance->state == FuriEventLoopStateStopped);
    furi_check(message_queue);

    FURI_CRITICAL_ENTER();

    furi_check(FuriEventLoopTree_get(instance->tree, message_queue) == NULL);

    // Allocate and setup item
    FuriEventLoopItem* item = furi_event_loop_item_alloc(
        instance, &furi_message_queue_event_loop_contract, message_queue, event);
    furi_event_loop_item_set_callback(item, callback, context);

    FuriEventLoopTree_set_at(instance->tree, message_queue, item);

    FuriEventLoopLink* link = item->contract->get_link(message_queue);

    if(item->event == FuriEventLoopEventIn) {
        furi_check(link->item_in == NULL);
        link->item_in = item;
    } else if(item->event == FuriEventLoopEventOut) {
        furi_check(link->item_out == NULL);
        link->item_out = item;
    } else {
        furi_crash();
    }

    if(item->contract->get_level(item->object, item->event)) {
        furi_event_loop_item_notify(item);
    }

    FURI_CRITICAL_EXIT();
}

void furi_event_loop_message_queue_unsubscribe(
    FuriEventLoop* instance,
    FuriMessageQueue* message_queue) {
    furi_check(instance);
    furi_check(instance->state == FuriEventLoopStateStopped);
    furi_check(instance->thread_id == furi_thread_get_current_id());

    FURI_CRITICAL_ENTER();

    FuriEventLoopItem** item_ptr = FuriEventLoopTree_get(instance->tree, message_queue);
    furi_check(item_ptr);

    FuriEventLoopItem* item = *item_ptr;
    furi_check(item);
    furi_check(item->owner == instance);

    FuriEventLoopLink* link = item->contract->get_link(message_queue);

    if(item->event == FuriEventLoopEventIn) {
        furi_check(link->item_in == item);
        link->item_in = NULL;
    } else if(item->event == FuriEventLoopEventOut) {
        furi_check(link->item_out == item);
        link->item_out = NULL;
    } else {
        furi_crash();
    }

    furi_event_loop_item_free(item);

    FuriEventLoopTree_erase(instance->tree, message_queue);

    FURI_CRITICAL_EXIT();
}

/* 
 * Event Loop Item API, used internally
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
    free(instance);
}

static void furi_event_loop_item_set_callback(
    FuriEventLoopItem* instance,
    FuriEventLoopMessageQueueCallback callback,
    void* callback_context) {
    furi_assert(instance);
    furi_assert(!instance->callback);

    instance->callback = callback;
    instance->callback_context = callback_context;
}

static void furi_event_loop_item_notify(FuriEventLoopItem* instance) {
    furi_assert(instance);

    FURI_CRITICAL_ENTER();

    if(!instance->WaitingList.prev && !instance->WaitingList.next) {
        WaitingList_push_back(instance->owner->waiting_list, instance);
    }

    FURI_CRITICAL_EXIT();

    xTaskNotifyIndexed(
        instance->owner->thread_id,
        FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
        FuriEventLoopFlagEvent,
        eSetBits);
}

void furi_event_loop_link_notify(FuriEventLoopLink* instance, FuriEventLoopEvent event) {
    furi_assert(instance);

    FURI_CRITICAL_ENTER();

    if(event == FuriEventLoopEventIn) {
        if(instance->item_in) furi_event_loop_item_notify(instance->item_in);
    } else if(event == FuriEventLoopEventOut) {
        if(instance->item_out) furi_event_loop_item_notify(instance->item_out);
    } else {
        furi_crash();
    }

    FURI_CRITICAL_EXIT();
}

bool furi_event_loop_signal_callback(uint32_t signal, void* arg, void* context) {
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
