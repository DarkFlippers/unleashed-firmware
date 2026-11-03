#include "ra_worker.h"

#include <furi.h>
#include <toolbox/level_duration.h>

#define TAG "RaWorker"

/** Edges the queue holds. See the note in the header for why this is not the firmware's 4096. */
#define RA_WORKER_QUEUE_SIZE 1024

/** Edges shorter than this are glued onto the previous one, as the firmware worker does. */
#define RA_WORKER_FILTER_US 30

struct RaWorker {
    FuriThread* thread;
    FuriStreamBuffer* stream;

    volatile bool running;
    volatile bool overrun;

    LevelDuration filter_level_duration;

    RaWorkerPairCallback pair_callback;
    RaWorkerOverrunCallback overrun_callback;
    void* context;
};

void ra_worker_rx_callback(bool level, uint32_t duration, void* context) {
    RaWorker* instance = context;

    LevelDuration level_duration = level_duration_make(level, duration);
    if(instance->overrun) {
        instance->overrun = false;
        level_duration = level_duration_reset();
    }
    if(furi_stream_buffer_send(instance->stream, &level_duration, sizeof(LevelDuration), 0) !=
       sizeof(LevelDuration)) {
        instance->overrun = true;
    }
}

static int32_t ra_worker_thread(void* context) {
    RaWorker* instance = context;
    LevelDuration level_duration;

    while(instance->running) {
        if(furi_stream_buffer_receive(
               instance->stream, &level_duration, sizeof(LevelDuration), 10) !=
           sizeof(LevelDuration)) {
            continue;
        }

        if(level_duration_is_reset(level_duration)) {
            FURI_LOG_W(TAG, "Overrun");
            if(instance->overrun_callback) instance->overrun_callback(instance->context);
            continue;
        }

        const bool level = level_duration_get_level(level_duration);
        const uint32_t duration = level_duration_get_duration(level_duration);

        if((duration < RA_WORKER_FILTER_US) || (instance->filter_level_duration.level == level)) {
            instance->filter_level_duration.duration += duration;
        } else {
            if(instance->pair_callback) {
                instance->pair_callback(
                    instance->context,
                    instance->filter_level_duration.level,
                    instance->filter_level_duration.duration);
            }
            instance->filter_level_duration.duration = duration;
            instance->filter_level_duration.level = level;
        }
    }

    return 0;
}

RaWorker* ra_worker_alloc(
    RaWorkerPairCallback pair_callback,
    RaWorkerOverrunCallback overrun_callback,
    void* context) {
    RaWorker* instance = malloc(sizeof(RaWorker));

    /* get_string() of the protocol that just decoded runs on this thread, down the
     * pair callback, so this stack carries the same load as the firmware worker's. */
    instance->thread = furi_thread_alloc_ex("RaWorker", 2048, ra_worker_thread, instance);
    instance->stream = furi_stream_buffer_alloc(
        sizeof(LevelDuration) * RA_WORKER_QUEUE_SIZE, sizeof(LevelDuration));

    instance->running = false;
    instance->overrun = false;
    instance->filter_level_duration = level_duration_make(false, 0);
    instance->pair_callback = pair_callback;
    instance->overrun_callback = overrun_callback;
    instance->context = context;

    return instance;
}

void ra_worker_free(RaWorker* instance) {
    furi_assert(instance);
    furi_check(!instance->running);

    furi_stream_buffer_free(instance->stream);
    furi_thread_free(instance->thread);
    free(instance);
}

void ra_worker_start(RaWorker* instance) {
    furi_assert(instance);
    furi_check(!instance->running);

    furi_stream_buffer_reset(instance->stream);
    instance->overrun = false;
    instance->filter_level_duration = level_duration_make(false, 0);
    instance->running = true;

    furi_thread_start(instance->thread);
}

void ra_worker_stop(RaWorker* instance) {
    furi_assert(instance);
    furi_check(instance->running);

    instance->running = false;
    furi_thread_join(instance->thread);
}

bool ra_worker_is_running(RaWorker* instance) {
    furi_assert(instance);
    return instance->running;
}
