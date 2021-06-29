#pragma once

#include <api-hal.h>

typedef struct SubGhzWorker SubGhzWorker;

typedef void (*SubGhzWorkerOverrunCallback)(void* context);

typedef void (*SubGhzWorkerPairCallback)(void* context, LevelPair pair);

void subghz_worker_rx_callback(ApiHalSubGhzCaptureLevel level, uint32_t duration, void* context);

SubGhzWorker* subghz_worker_alloc();

void subghz_worker_free(SubGhzWorker* instance);

void subghz_worker_set_overrun_callback(SubGhzWorker* instance, SubGhzWorkerOverrunCallback callback);

void subghz_worker_set_pair_callback(SubGhzWorker* instance, SubGhzWorkerPairCallback callback);

void subghz_worker_set_context(SubGhzWorker* instance, void* context);

void subghz_worker_start(SubGhzWorker* instance);

void subghz_worker_stop(SubGhzWorker* instance);
