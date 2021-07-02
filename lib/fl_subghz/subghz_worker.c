#include "subghz_worker.h"

#include <stream_buffer.h>
#include <furi.h>

struct SubGhzWorker {
    FuriThread* thread;
    StreamBufferHandle_t stream;

    volatile bool running;
    volatile bool overrun;

    SubGhzWorkerOverrunCallback overrun_callback;
    SubGhzWorkerPairCallback pair_callback;
    void* context;
};

/** Rx callback timer
 * 
 * @param level received signal level
 * @param duration received signal duration
 * @param context 
 */
void subghz_worker_rx_callback(
    ApiHalSubGhzCaptureLevel level,
    uint32_t duration,
    void* context) {

    SubGhzWorker* instance = context;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    LevelPair pair = {.level = level, .duration = duration};
    if(instance->overrun) {
        instance->overrun = false;
        pair.level = ApiHalSubGhzCaptureLevelOverrun;
    }
    size_t ret =
        xStreamBufferSendFromISR(instance->stream, &pair, sizeof(LevelPair), &xHigherPriorityTaskWoken);
    if(sizeof(LevelPair) != ret) instance->overrun = true;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/** Worker callback thread
 * 
 * @param context 
 * @return exit code 
 */
static int32_t subghz_worker_thread_callback(void* context) {
    SubGhzWorker* instance = context;

    LevelPair pair;
    while(instance->running) {
        int ret = xStreamBufferReceive(instance->stream, &pair, sizeof(LevelPair), 10);
        if(ret == sizeof(LevelPair)) {
            if(pair.level == ApiHalSubGhzCaptureLevelOverrun) {
                printf(".");
                if (instance->overrun_callback) instance->overrun_callback(instance->context);
            } else {
                if (instance->pair_callback) instance->pair_callback(instance->context, pair);
            }
        }
    }

    return 0;
}

SubGhzWorker* subghz_worker_alloc() {
    SubGhzWorker* instance = furi_alloc(sizeof(SubGhzWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "subghz_worker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subghz_worker_thread_callback);
    
    instance->stream = xStreamBufferCreate(sizeof(LevelPair) * 1024, sizeof(LevelPair));

    return instance;
}

void subghz_worker_free(SubGhzWorker* instance) {
    furi_assert(instance);

    vStreamBufferDelete(instance->stream);
    furi_thread_free(instance->thread);

    free(instance);
}

void subghz_worker_set_overrun_callback(SubGhzWorker* instance, SubGhzWorkerOverrunCallback callback) {
    furi_assert(instance);
    instance->overrun_callback = callback;
}

void subghz_worker_set_pair_callback(SubGhzWorker* instance, SubGhzWorkerPairCallback callback) {
    furi_assert(instance);
    instance->pair_callback = callback;
}

void subghz_worker_set_context(SubGhzWorker* instance, void* context) {
    furi_assert(instance);
    instance->context = context;
}

void subghz_worker_start(SubGhzWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->running);

    instance->running = true;

    furi_thread_start(instance->thread);
}

void subghz_worker_stop(SubGhzWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->running);

    instance->running = false;

    furi_thread_join(instance->thread);
}
