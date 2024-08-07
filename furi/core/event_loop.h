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

/**
 * @brief Enumeration of event types, flags and masks.
 *
 * Only one event direction (In or Out) can be used per subscription.
 * An object can have no more than one subscription for each direction.
 *
 * Additional flags that modify the behaviour can be
 * set using the bitwise OR operation (see flag description).
 */
typedef enum {
    /**
     * @brief Subscribe to In events.
     *
     * In events occur on the following conditions:
     * - One or more items were inserted into a FuriMessageQueue,
     * - Enough data has been written to a FuriStreamBuffer,
     * - A FuriSemaphore has been released at least once,
     * - A FuriMutex has been released.
     */
    FuriEventLoopEventIn = 0x00000001U,
    /**
     * @brief Subscribe to Out events.
     *
     * Out events occur on the following conditions:
     * - One or more items were removed from a FuriMessageQueue,
     * - Any amount of data has been read out of a FuriStreamBuffer,
     * - A FuriSemaphore has been acquired at least once,
     * - A FuriMutex has been acquired.
     */
    FuriEventLoopEventOut = 0x00000002U,
    /**
     * @brief Special value containing the event direction bits, used internally.
     */
    FuriEventLoopEventMask = 0x00000003U,
    /**
     * @brief Use edge triggered events.
     *
     * By default, level triggered events are used. A level above zero
     * is reported based on the following conditions:
     *
     * In events:
     * - a FuriMessageQueue contains one or more items,
     * - a FuriStreamBuffer contains one or more bytes,
     * - a FuriSemaphore can be acquired at least once,
     * - a FuriMutex can be acquired.
     *
     * Out events:
     * - a FuriMessageQueue has at least one item of free space,
     * - a FuriStreamBuffer has at least one byte of free space,
     * - a FuriSemaphore has been acquired at least once,
     * - a FuriMutex has been acquired.
     *
     * If this flag is NOT set, the event will be generated repeatedly until
     * the level becomes zero (e.g. all items have been removed from
     * a FuriMessageQueue in case of the "In" event, etc.)
     *
     * If this flag IS set, then the above check is skipped and the event
     * is generated ONLY when a change occurs, with the event direction
     * (In or Out) taken into account.
     */
    FuriEventLoopEventFlagEdge = 0x00000004U,
    /**
     * @brief Automatically unsubscribe from events after one time.
     *
     * By default, events will be generated each time the specified conditions
     * have been met. If this flag IS set, the event subscription will be cancelled
     * upon the first occurred event and no further events will be generated.
     */
    FuriEventLoopEventFlagOnce = 0x00000008U,
    /**
     * @brief Special value containing the event flag bits, used internally.
     */
    FuriEventLoopEventFlagMask = 0xFFFFFFFCU,
    /**
     * @brief Special value to force the enum to 32-bit values.
     */
    FuriEventLoopEventReserved = UINT32_MAX,
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
 * Event subscription/notification APIs
 */

typedef void FuriEventLoopObject;

/** Callback type for event loop events
 *
 * @param      object   The object that triggered the event
 * @param      context  The context that was provided upon subscription
 *
 * @return     true if event was processed, false if we need to delay processing
 */
typedef bool (*FuriEventLoopEventCallback)(FuriEventLoopObject* object, void* context);

/** Opaque message queue type */
typedef struct FuriMessageQueue FuriMessageQueue;

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
void furi_event_loop_subscribe_message_queue(
    FuriEventLoop* instance,
    FuriMessageQueue* message_queue,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context);

/** Opaque stream buffer type */
typedef struct FuriStreamBuffer FuriStreamBuffer;

/** Subscribe to stream buffer events
 *
 * @warning you can only have one subscription for one event type.
 *
 * @param      instance       The Event Loop instance
 * @param      stream_buffer  The stream buffer to add
 * @param[in]  event          The Event Loop event to trigger on
 * @param[in]  callback       The callback to call on event
 * @param      context        The context for callback
 */
void furi_event_loop_subscribe_stream_buffer(
    FuriEventLoop* instance,
    FuriStreamBuffer* stream_buffer,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context);

/** Opaque semaphore type */
typedef struct FuriSemaphore FuriSemaphore;

/** Subscribe to semaphore events
 *
 * @warning you can only have one subscription for one event type.
 *
 * @param      instance       The Event Loop instance
 * @param      semaphore      The semaphore to add
 * @param[in]  event          The Event Loop event to trigger on
 * @param[in]  callback       The callback to call on event
 * @param      context        The context for callback
 */
void furi_event_loop_subscribe_semaphore(
    FuriEventLoop* instance,
    FuriSemaphore* semaphore,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context);

/** Opaque mutex type */
typedef struct FuriMutex FuriMutex;

/** Subscribe to mutex events
 *
 * @warning you can only have one subscription for one event type.
 *
 * @param      instance       The Event Loop instance
 * @param      mutex          The mutex to add
 * @param[in]  event          The Event Loop event to trigger on
 * @param[in]  callback       The callback to call on event
 * @param      context        The context for callback
 */
void furi_event_loop_subscribe_mutex(
    FuriEventLoop* instance,
    FuriMutex* mutex,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context);

/** Unsubscribe from events (common)
 *
 * @param      instance       The Event Loop instance
 * @param      object         The object to unsubscribe from
 */
void furi_event_loop_unsubscribe(FuriEventLoop* instance, FuriEventLoopObject* object);

#ifdef __cplusplus
}
#endif
