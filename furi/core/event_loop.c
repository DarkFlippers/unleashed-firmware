#include "event_loop_i.h"
#include "message_queue_i.h"

#include "check.h"
#include "thread.h"

#include <m-bptree.h>
#include <m-i-list.h>

#include <FreeRTOS.h>
#include <task.h>

struct FuriEventLoopItem {
    // Source
    FuriEventLoop* owner;

    // Tracking item
    const FuriEventLoopContract* contract;
    void* object;
    FuriEventLoopEvent event;

    // Callback and context
    FuriEventLoopMessageQueueCallback callback;
    void* callback_context;

    // Waiting list
    ILIST_INTERFACE(WaitingList, struct FuriEventLoopItem);
};

ILIST_DEF(WaitingList, FuriEventLoopItem, M_POD_OPLIST)

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

/* Event Loop RB tree */
#define FURI_EVENT_LOOP_TREE_RANK (4)

BPTREE_DEF2( // NOLINT
    FuriEventLoopTree,
    FURI_EVENT_LOOP_TREE_RANK,
    void*, /* pointer to object we track */
    M_PTR_OPLIST,
    FuriEventLoopItem*, /* pointer to the FuriEventLoopItem */
    M_PTR_OPLIST)

#define M_OPL_FuriEventLoopTree_t() BPTREE_OPLIST(FuriEventLoopTree, M_POD_OPLIST)

#define FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX (2)

typedef enum {
    FuriEventLoopFlagEvent = (1 << 0),
    FuriEventLoopFlagStop = (1 << 1),
} FuriEventLoopFlag;

#define FuriEventLoopFlagAll (FuriEventLoopFlagEvent | FuriEventLoopFlagStop)

typedef enum {
    FuriEventLoopProcessStatusComplete,
    FuriEventLoopProcessStatusIncomplete,
    FuriEventLoopProcessStatusAgain,
} FuriEventLoopProcessStatus;

typedef enum {
    FuriEventLoopStateIdle,
    FuriEventLoopStateProcessing,
} FuriEventLoopState;

struct FuriEventLoop {
    // Only works if all operations are done from the same thread
    FuriThreadId thread_id;

    // Poller state
    volatile FuriEventLoopState state;

    // Tree
    FuriEventLoopTree_t tree;
    // Tree waiting list
    WaitingList_t waiting_list;

    // Tick event
    uint32_t tick_interval;
    FuriEventLoopTickCallback tick_callback;
    void* tick_callback_context;
};

FuriEventLoop* furi_event_loop_alloc(void) {
    FuriEventLoop* instance = malloc(sizeof(FuriEventLoop));

    instance->thread_id = furi_thread_get_current_id();
    FuriEventLoopTree_init(instance->tree);
    WaitingList_init(instance->waiting_list);

    return instance;
}

void furi_event_loop_free(FuriEventLoop* instance) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());

    FuriEventLoopTree_clear(instance->tree);
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

void furi_event_loop_run(FuriEventLoop* instance) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());

    uint32_t timeout = instance->tick_callback ? instance->tick_interval : FuriWaitForever;

    while(true) {
        uint32_t flags = 0;
        BaseType_t ret = xTaskNotifyWaitIndexed(
            FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, 0, FuriEventLoopFlagAll, &flags, timeout);

        instance->state = FuriEventLoopStateProcessing;
        if(ret == pdTRUE) {
            if(flags & FuriEventLoopFlagStop) {
                instance->state = FuriEventLoopStateIdle;
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
            }
        } else {
            if(instance->tick_callback) {
                instance->tick_callback(instance->tick_callback_context);
            }
        }
        instance->state = FuriEventLoopStateIdle;
    }
}

void furi_event_loop_stop(FuriEventLoop* instance) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());

    xTaskNotifyIndexed(
        instance->thread_id, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, FuriEventLoopFlagStop, eSetBits);
}

void furi_event_loop_tick_set(
    FuriEventLoop* instance,
    uint32_t interval,
    FuriEventLoopTickCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(callback ? interval > 0 : true);

    instance->tick_interval = interval;
    instance->tick_callback = callback;
    instance->tick_callback_context = context;
}

void furi_event_loop_message_queue_subscribe(
    FuriEventLoop* instance,
    FuriMessageQueue* message_queue,
    FuriEventLoopEvent event,
    FuriEventLoopMessageQueueCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(instance->state == FuriEventLoopStateIdle);
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
    furi_check(instance->state == FuriEventLoopStateIdle);
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