/**
 * @file event_loop_timer.h
 * @brief Software timer functionality for FuriEventLoop.
 */

#pragma once

#include "event_loop.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of possible timer types.
 */
typedef enum {
    FuriEventLoopTimerTypeOnce = 0, /**< One-shot timer. */
    FuriEventLoopTimerTypePeriodic = 1, /**< Repeating timer. */
} FuriEventLoopTimerType;

/**
 * @brief Timer callback type for functions to be called when a timer expires.
 *
 * In the timer callback, it is ALLOWED:
 * - To start, stop, or restart an existing timer,
 * - To create new timers using furi_event_loop_timer_alloc(),
 * - To delete timers using furi_event_loop_timer_free().
 *
 * @param[in,out] context pointer to a user-specific object that was provided during timer creation
 */
typedef void (*FuriEventLoopTimerCallback)(void* context);

/**
 * @brief Opaque event loop timer type.
 */
typedef struct FuriEventLoopTimer FuriEventLoopTimer;

/**
 * @brief Create a new event loop timer instance.
 *
 * @param[in,out] instance pointer to the current FuriEventLoop instance
 * @param[in] callback pointer to the callback function to be executed upon timer timeout
 * @param[in] type timer type value to determine its behavior (single-shot or periodic)
 * @param[in,out] context pointer to a user-specific object (will be passed to the callback)
 * @returns pointer to the created timer instance
 */
FuriEventLoopTimer* furi_event_loop_timer_alloc(
    FuriEventLoop* instance,
    FuriEventLoopTimerCallback callback,
    FuriEventLoopTimerType type,
    void* context);

/**
 * @brief Delete an event loop timer instance.
 *
 * @warning The user code MUST call furi_event_loop_timer_free() on ALL instances
 *          associated with the current event loop BEFORE calling furi_event_loop_free().
 *          The event loop may EITHER be running OR stopped when the timers are being deleted.
 *
 * @param[in,out] timer pointer to the timer instance to be deleted
 */
void furi_event_loop_timer_free(FuriEventLoopTimer* timer);

/**
 * @brief Start a timer or restart it with a new interval.
 *
 * @param[in,out] timer pointer to the timer instance to be (re)started
 * @param[in] interval timer interval in ticks
 */
void furi_event_loop_timer_start(FuriEventLoopTimer* timer, uint32_t interval);

/**
 * @brief Restart a timer with the previously set interval.
 *
 * @param[in,out] timer pointer to the timer instance to be restarted
 */
void furi_event_loop_timer_restart(FuriEventLoopTimer* timer);

/**
 * @brief Stop a timer without firing its callback.
 *
 * It is safe to call this function on an already stopped timer (it will do nothing).
 *
 * @param[in,out] timer pointer to the timer instance to be stopped
 */
void furi_event_loop_timer_stop(FuriEventLoopTimer* timer);

/**
 * @brief Get the time remaining before the timer becomes expires.
 *
 * For stopped or expired timers, this function returns 0.
 *
 * @param[in] timer pointer to the timer to be queried
 * @returns remaining time in ticks
 */
uint32_t furi_event_loop_timer_get_remaining_time(const FuriEventLoopTimer* timer);

/**
 * @brief Get the timer interval.
 *
 * @param[in] timer pointer to the timer to be queried
 * @returns timer interval in ticks
 */
uint32_t furi_event_loop_timer_get_interval(const FuriEventLoopTimer* timer);

/**
 * @brief Check if the timer is currently running.
 *
 * A timer is considered running if it has not expired yet.
 * @param[in] timer pointer to the timer to be queried
 * @returns true if the timer is running, false otherwise
 */
bool furi_event_loop_timer_is_running(const FuriEventLoopTimer* timer);

#ifdef __cplusplus
}
#endif
