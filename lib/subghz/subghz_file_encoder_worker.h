#pragma once

#include <furi-hal.h>

typedef struct SubGhzFileEncoderWorker SubGhzFileEncoderWorker;

/** Allocate SubGhzFileEncoderWorker
 * 
 * @return SubGhzFileEncoderWorker* 
 */
SubGhzFileEncoderWorker* subghz_file_encoder_worker_alloc();

/** Free SubGhzFileEncoderWorker
 * 
 * @param instance SubGhzFileEncoderWorker instance
 */
void subghz_file_encoder_worker_free(SubGhzFileEncoderWorker* instance);

LevelDuration subghz_file_encoder_worker_get_level_duration(void* context);

/** Start SubGhzFileEncoderWorker
 * 
 * @param instance SubGhzFileEncoderWorker instance
 * @return bool - true if ok
 */
bool subghz_file_encoder_worker_start(SubGhzFileEncoderWorker* instance, const char* file_path);

/** Stop SubGhzFileEncoderWorker
 * 
 * @param instance SubGhzFileEncoderWorker instance
 */
void subghz_file_encoder_worker_stop(SubGhzFileEncoderWorker* instance);

/** Check if worker is running
 * 
 * @param instance SubGhzFileEncoderWorker instance
 * @return bool - true if running
 */
bool subghz_file_encoder_worker_is_running(SubGhzFileEncoderWorker* instance);
