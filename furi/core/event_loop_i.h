#pragma once

#include "event_loop.h"
#include "event_loop_link_i.h"
#include "event_loop_timer_i.h"
#include "event_loop_tick_i.h"
#include "event_loop_thread_flag_interface.h"

#include <m-list.h>
#include <m-bptree.h>
#include <m-i-list.h>

#include "thread.h"
#include "thread_i.h"

struct FuriEventLoopItem {
    // Source
    FuriEventLoop* owner;

    // Tracking item
    FuriEventLoopEvent event;
    FuriEventLoopObject* object;
    const FuriEventLoopContract* contract;

    // Callback and context
    FuriEventLoopEventCallback callback;
    void* callback_context;

    // Waiting list
    ILIST_INTERFACE(WaitingList, FuriEventLoopItem);
};

ILIST_DEF(WaitingList, FuriEventLoopItem, M_POD_OPLIST)

/* Event Loop RB tree */
#define FURI_EVENT_LOOP_TREE_RANK (4)

BPTREE_DEF2( // NOLINT
    FuriEventLoopTree,
    FURI_EVENT_LOOP_TREE_RANK,
    FuriEventLoopObject*, /* pointer to object we track */
    M_PTR_OPLIST,
    FuriEventLoopItem*, /* pointer to the FuriEventLoopItem */
    M_PTR_OPLIST)

#define M_OPL_FuriEventLoopTree_t() BPTREE_OPLIST(FuriEventLoopTree, M_POD_OPLIST)

#define FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX (2)

typedef enum {
    FuriEventLoopFlagEvent = (1 << 0),
    FuriEventLoopFlagStop = (1 << 1),
    FuriEventLoopFlagTimer = (1 << 2),
    FuriEventLoopFlagPending = (1 << 3),
    FuriEventLoopFlagThreadFlag = (1 << 4),
} FuriEventLoopFlag;

#define FuriEventLoopFlagAll                                                   \
    (FuriEventLoopFlagEvent | FuriEventLoopFlagStop | FuriEventLoopFlagTimer | \
     FuriEventLoopFlagPending | FuriEventLoopFlagThreadFlag)

typedef enum {
    FuriEventLoopProcessStatusComplete,
    FuriEventLoopProcessStatusIncomplete,
    FuriEventLoopProcessStatusFreeLater,
} FuriEventLoopProcessStatus;

typedef enum {
    FuriEventLoopStateStopped,
    FuriEventLoopStateRunning,
} FuriEventLoopState;

typedef struct {
    FuriEventLoopPendingCallback callback;
    void* context;
} FuriEventLoopPendingQueueItem;

LIST_DUAL_PUSH_DEF(PendingQueue, FuriEventLoopPendingQueueItem, M_POD_OPLIST)

struct FuriEventLoop {
    // Only works if all operations are done from the same thread
    FuriThreadId thread_id;

    // Poller state
    volatile FuriEventLoopState state;
    volatile FuriEventLoopItem* current_item;

    // Event handling
    FuriEventLoopTree_t tree;
    WaitingList_t waiting_list;

    // Active timer list
    TimerList_t timer_list;
    // Timer request queue
    TimerQueue_t timer_queue;
    // Pending callback queue
    PendingQueue_t pending_queue;
    // Tick event
    FuriEventLoopTick tick;

    // Thread flags callback
    bool are_thread_flags_subscribed;
    FuriEventLoopThreadFlagsCallback thread_flags_callback;
    void* thread_flags_callback_context;
};
