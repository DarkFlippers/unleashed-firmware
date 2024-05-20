#pragma once

#include <furi_hal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SubGhzFileEncoderWorkerCallbackEnd)(void* context);

typedef struct SubGhzFileEncoderWorker SubGhzFileEncoderWorker;

/** 
 * End callback SubGhzWorker.
 * @param instance SubGhzFileEncoderWorker instance
 * @param callback SubGhzFileEncoderWorkerCallbackEnd callback
 */
void subghz_file_encoder_worker_callback_end(
    SubGhzFileEncoderWorker* instance,
    SubGhzFileEncoderWorkerCallbackEnd callback_end,
    void* context_end);

/** 
 * Allocate SubGhzFileEncoderWorker.
 * @return SubGhzFileEncoderWorker* pointer to a SubGhzFileEncoderWorker instance 
 */
SubGhzFileEncoderWorker* subghz_file_encoder_worker_alloc(void);

/** 
 * Free SubGhzFileEncoderWorker.
 * @param instance Pointer to a SubGhzFileEncoderWorker instance
 */
void subghz_file_encoder_worker_free(SubGhzFileEncoderWorker* instance);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzFileEncoderWorker instance
 * @return LevelDuration 
 */
LevelDuration subghz_file_encoder_worker_get_level_duration(void* context);

/** 
 * Start SubGhzFileEncoderWorker.
 * @param instance Pointer to a SubGhzFileEncoderWorker instance
 * @param file_path File path
 * @param radio_device_name Radio device name
 * @return bool - true if ok
 */
bool subghz_file_encoder_worker_start(
    SubGhzFileEncoderWorker* instance,
    const char* file_path,
    const char* radio_device_name);

/** 
 * Stop SubGhzFileEncoderWorker
 * @param instance Pointer to a SubGhzFileEncoderWorker instance
 */
void subghz_file_encoder_worker_stop(SubGhzFileEncoderWorker* instance);

/** 
 * Check if worker is running
 * @param instance Pointer to a SubGhzFileEncoderWorker instance
 * @return bool - true if running
 */
bool subghz_file_encoder_worker_is_running(SubGhzFileEncoderWorker* instance);

#ifdef __cplusplus
}
#endif
