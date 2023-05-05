#pragma once

#include "core/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*FuriTimerCallback)(void* context);

typedef enum {
    FuriTimerTypeOnce = 0, ///< One-shot timer.
    FuriTimerTypePeriodic = 1 ///< Repeating timer.
} FuriTimerType;

typedef void FuriTimer;

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

/** Start timer
 *
 * @param      instance  The pointer to FuriTimer instance
 * @param[in]  ticks     The ticks
 *
 * @return     The furi status.
 */
FuriStatus furi_timer_start(FuriTimer* instance, uint32_t ticks);

/** Stop timer
 *
 * @param      instance  The pointer to FuriTimer instance
 *
 * @return     The furi status.
 */
FuriStatus furi_timer_stop(FuriTimer* instance);

/** Is timer running
 *
 * @param      instance  The pointer to FuriTimer instance
 *
 * @return     0: not running, 1: running
 */
uint32_t furi_timer_is_running(FuriTimer* instance);

typedef void (*FuriTimerPendigCallback)(void* context, uint32_t arg);

void furi_timer_pending_callback(FuriTimerPendigCallback callback, void* context, uint32_t arg);

#ifdef __cplusplus
}
#endif
