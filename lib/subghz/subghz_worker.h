#pragma once

#include <furi_hal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SubGhzWorker SubGhzWorker;

typedef void (*SubGhzWorkerOverrunCallback)(void* context);

typedef void (*SubGhzWorkerPairCallback)(void* context, bool level, uint32_t duration);

void subghz_worker_rx_callback(bool level, uint32_t duration, void* context);

/** 
 * Allocate SubGhzWorker.
 * @return SubGhzWorker* Pointer to a SubGhzWorker instance
 */
SubGhzWorker* subghz_worker_alloc();

/** 
 * Free SubGhzWorker.
 * @param instance Pointer to a SubGhzWorker instance
 */
void subghz_worker_free(SubGhzWorker* instance);

/** 
 * Overrun callback SubGhzWorker.
 * @param instance Pointer to a SubGhzWorker instance
 * @param callback SubGhzWorkerOverrunCallback callback
 */
void subghz_worker_set_overrun_callback(
    SubGhzWorker* instance,
    SubGhzWorkerOverrunCallback callback);

/** 
 * Pair callback SubGhzWorker.
 * @param instance Pointer to a SubGhzWorker instance
 * @param callback SubGhzWorkerOverrunCallback callback
 */
void subghz_worker_set_pair_callback(SubGhzWorker* instance, SubGhzWorkerPairCallback callback);

/** 
 * Context callback SubGhzWorker.
 * @param instance Pointer to a SubGhzWorker instance
 * @param context 
 */
void subghz_worker_set_context(SubGhzWorker* instance, void* context);

/** 
 * Start SubGhzWorker.
 * @param instance Pointer to a SubGhzWorker instance
 */
void subghz_worker_start(SubGhzWorker* instance);

/** Stop SubGhzWorker
 * @param instance Pointer to a SubGhzWorker instance
 */
void subghz_worker_stop(SubGhzWorker* instance);

/** 
 * Check if worker is running.
 * @param instance Pointer to a SubGhzWorker instance
 * @return bool - true if running
 */
bool subghz_worker_is_running(SubGhzWorker* instance);

/** 
 * Short duration filter setting.
 * glues short durations into 1. The default setting is 30 us, if set to 0 the filter will be disabled
 * @param instance Pointer to a SubGhzWorker instance
 * @param timeout time in us
 */
void subghz_worker_set_filter(SubGhzWorker* instance, uint16_t timeout);

#ifdef __cplusplus
}
#endif
