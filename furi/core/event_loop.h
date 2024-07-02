/**
 * @file event_loop.h
 * @brief      Furi Event Loop
 *
 *             This module is designed to handle application event loop in fully
 *             asynchronous, reactive nature. On the low level this modules is
 *             inspired by epoll/kqueue concept, on the high level by asyncio
 *             event loop.
 *
 *             This module is trying to best fit into Furi OS, so we don't
 *             provide any compatibility with other event driven APIs. But
 *             programming concepts are the same, except some runtime
 *             limitations from our side.
 */
#pragma once

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Event Loop events */
typedef enum {
    FuriEventLoopEventOut, /**< On departure: item was retrieved from container, flag reset, etc... */
    FuriEventLoopEventIn, /**< On arrival: item was inserted into container, flag set, etc... */
} FuriEventLoopEvent;

/** Anonymous message queue type */
typedef struct FuriEventLoop FuriEventLoop;

/** Allocate Event Loop instance
 *
 * Couple things to keep in mind:
 * - You can have 1 event_loop per 1 thread
 * - You can not use event_loop instance in the other thread
 * - Do not use blocking API to query object delegated to Event Loop
 *
 * @return     The Event Loop instance
 */
FuriEventLoop* furi_event_loop_alloc(void);

/** Free Event Loop instance
 *
 * @param      instance  The Event Loop instance
 */
void furi_event_loop_free(FuriEventLoop* instance);

/** Continuously poll for events
 *
 * Can be stopped with `furi_event_loop_stop`
 *
 * @param      instance  The Event Loop instance
 */
void furi_event_loop_run(FuriEventLoop* instance);

/** Stop Event Loop instance
 *
 * @param      instance  The Event Loop instance
 */
void furi_event_loop_stop(FuriEventLoop* instance);

/*
 * Tick related API
 */

/** Tick callback type
 *
 * @param      context  The context for callback
 */
typedef void (*FuriEventLoopTickCallback)(void* context);

/** Set Event Loop tick callback
 *
 * Tick callback is called periodically after specified inactivity time.
 * It acts like a low-priority timer: it will only fire if there is time
 * left after processing the synchronization primitives and the regular timers.
 * Therefore, it is not monotonic: ticks will be skipped if the event loop is busy.
 *
 * @param      instance  The Event Loop instance
 * @param[in]  interval  The tick interval
 * @param[in]  callback  The callback to call
 * @param      context   The context for callback
 */
void furi_event_loop_tick_set(
    FuriEventLoop* instance,
    uint32_t interval,
    FuriEventLoopTickCallback callback,
    void* context);

/*
 * Deferred function call API
 */

/**
 * @brief Timer callback type for functions to be called in a deferred manner.
 *
 * @param[in,out] context pointer to a user-specific object that was provided during
 *                        furi_event_loop_pend_callback() call
 */
typedef void (*FuriEventLoopPendingCallback)(void* context);

/**
 * @brief Call a function when all preceding timer commands are processed
 *
 * This function may be useful to call another function when the event loop has been started.
 *
 * @param[in,out] instance pointer to the current FuriEventLoop instance
 * @param[in] callback pointer to the callback to be executed when previous commands have been processed
 * @param[in,out] context pointer to a user-specific object (will be passed to the callback)
 */
void furi_event_loop_pend_callback(
    FuriEventLoop* instance,
    FuriEventLoopPendingCallback callback,
    void* context);

/*
 * Message queue related APIs
 */

/** Anonymous message queue type */
typedef struct FuriMessageQueue FuriMessageQueue;

/** Callback type for message queue
 *
 * @param      queue    The queue that triggered event
 * @param      context  The context that was provided on
 *                      furi_event_loop_message_queue_subscribe call
 *
 * @return     true if event was processed, false if we need to delay processing
 */
typedef bool (*FuriEventLoopMessageQueueCallback)(FuriMessageQueue* queue, void* context);

/** Subscribe to message queue events
 * 
 * @warning you can only have one subscription for one event type.
 *
 * @param      instance       The Event Loop instance
 * @param      message_queue  The message queue to add
 * @param[in]  event          The Event Loop event to trigger on
 * @param[in]  callback       The callback to call on event
 * @param      context        The context for callback
 */
void furi_event_loop_message_queue_subscribe(
    FuriEventLoop* instance,
    FuriMessageQueue* message_queue,
    FuriEventLoopEvent event,
    FuriEventLoopMessageQueueCallback callback,
    void* context);

/** Unsubscribe from message queue
 *
 * @param      instance       The Event Loop instance
 * @param      message_queue  The message queue
 */
void furi_event_loop_message_queue_unsubscribe(
    FuriEventLoop* instance,
    FuriMessageQueue* message_queue);

#ifdef __cplusplus
}
#endif
