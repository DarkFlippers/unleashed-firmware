#include "subghz_worker.h"

#include <stream_buffer.h>
#include <furi.h>

#define TAG "SubGhzWorker"

struct SubGhzWorker {
    FuriThread* thread;
    StreamBufferHandle_t stream;

    volatile bool running;
    volatile bool overrun;

    LevelDuration filter_level_duration;
    bool filter_running;
    uint16_t filter_duration;

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
void subghz_worker_rx_callback(bool level, uint32_t duration, void* context) {
    SubGhzWorker* instance = context;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    LevelDuration level_duration = level_duration_make(level, duration);
    if(instance->overrun) {
        instance->overrun = false;
        level_duration = level_duration_reset();
    }
    size_t ret = xStreamBufferSendFromISR(
        instance->stream, &level_duration, sizeof(LevelDuration), &xHigherPriorityTaskWoken);
    if(sizeof(LevelDuration) != ret) instance->overrun = true;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/** Worker callback thread
 * 
 * @param context 
 * @return exit code 
 */
static int32_t subghz_worker_thread_callback(void* context) {
    SubGhzWorker* instance = context;

    LevelDuration level_duration;
    while(instance->running) {
        int ret =
            xStreamBufferReceive(instance->stream, &level_duration, sizeof(LevelDuration), 10);
        if(ret == sizeof(LevelDuration)) {
            if(level_duration_is_reset(level_duration)) {
                FURI_LOG_E(TAG, "Overrun buffer");
                if(instance->overrun_callback) instance->overrun_callback(instance->context);
            } else {
                bool level = level_duration_get_level(level_duration);
                uint32_t duration = level_duration_get_duration(level_duration);

                if(instance->filter_running) {
                    if((duration < instance->filter_duration) ||
                       (instance->filter_level_duration.level == level)) {
                        instance->filter_level_duration.duration += duration;

                    } else if(instance->filter_level_duration.level != level) {
                        if(instance->pair_callback)
                            instance->pair_callback(
                                instance->context,
                                instance->filter_level_duration.level,
                                instance->filter_level_duration.duration);

                        instance->filter_level_duration.duration = duration;
                        instance->filter_level_duration.level = level;
                    }
                } else {
                    if(instance->pair_callback)
                        instance->pair_callback(instance->context, level, duration);
                }
            }
        }
    }

    return 0;
}

SubGhzWorker* subghz_worker_alloc() {
    SubGhzWorker* instance = malloc(sizeof(SubGhzWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "SubGhzWorker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subghz_worker_thread_callback);

    instance->stream = xStreamBufferCreate(sizeof(LevelDuration) * 4096, sizeof(LevelDuration));

    //setting filter
    instance->filter_running = true;
    instance->filter_duration = 30;

    return instance;
}

void subghz_worker_free(SubGhzWorker* instance) {
    furi_assert(instance);

    vStreamBufferDelete(instance->stream);
    furi_thread_free(instance->thread);

    free(instance);
}

void subghz_worker_set_overrun_callback(
    SubGhzWorker* instance,
    SubGhzWorkerOverrunCallback callback) {
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

bool subghz_worker_is_running(SubGhzWorker* instance) {
    furi_assert(instance);
    return instance->running;
}
