/**
 * @file timer.h
 * @brief Furi software Timer API.
 */
#pragma once

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*FuriTimerCallback)(void* context);

typedef enum {
    FuriTimerTypeOnce = 0, ///< One-shot timer.
    FuriTimerTypePeriodic = 1 ///< Repeating timer.
} FuriTimerType;

typedef struct FuriTimer FuriTimer;

/** Allocate timer
 *
 * @param[in]  func     The callback function
 * @param[in]  type     The timer type
 * @param      context  The callback context
 *
 * @return     The pointer to FuriTimer instance
 */
FuriTimer* furi_timer_alloc(FuriTimerCallback func, FuriTimerType type, void* context);

/** Free timer
 *
 * @param      instance  The pointer to FuriTimer instance
 */
void furi_timer_free(FuriTimer* instance);

/** Flush timer task control message queue
 *
 * Ensures that all commands before this point was processed.
 */
void furi_timer_flush(void);

/** Start timer
 *
 * @warning    This is asynchronous call, real operation will happen as soon as
 *             timer service process this request.
 *
 * @param      instance  The pointer to FuriTimer instance
 * @param[in]  ticks     The interval in ticks
 *
 * @return     The furi status.
 */
FuriStatus furi_timer_start(FuriTimer* instance, uint32_t ticks);

/** Restart timer with previous timeout value
 *
 * @warning    This is asynchronous call, real operation will happen as soon as
 *             timer service process this request.
 *
 * @param      instance  The pointer to FuriTimer instance
 * @param[in]  ticks     The interval in ticks
 *
 * @return     The furi status.
 */
FuriStatus furi_timer_restart(FuriTimer* instance, uint32_t ticks);

/** Stop timer
 *
 * @warning    This is synchronous call that will be blocked till timer queue processed.
 *
 * @param      instance  The pointer to FuriTimer instance
 *
 * @return     The furi status.
 */
FuriStatus furi_timer_stop(FuriTimer* instance);

/** Is timer running
 *
 * @warning    This cal may and will return obsolete timer state if timer
 *             commands are still in the queue. Please read FreeRTOS timer
 *             documentation first.
 *
 * @param      instance  The pointer to FuriTimer instance
 *
 * @return     0: not running, 1: running
 */
uint32_t furi_timer_is_running(FuriTimer* instance);

/** Get timer expire time
 *
 * @param      instance  The Timer instance
 *
 * @return     expire tick
 */
uint32_t furi_timer_get_expire_time(FuriTimer* instance);

typedef void (*FuriTimerPendigCallback)(void* context, uint32_t arg);

void furi_timer_pending_callback(FuriTimerPendigCallback callback, void* context, uint32_t arg);

typedef enum {
    FuriTimerThreadPriorityNormal, /**< Lower then other threads */
    FuriTimerThreadPriorityElevated, /**< Same as other threads */
} FuriTimerThreadPriority;

/** Set Timer thread priority
 *
 * @param[in]  priority  The priority
 */
void furi_timer_set_thread_priority(FuriTimerThreadPriority priority);

#ifdef __cplusplus
}
#endif
