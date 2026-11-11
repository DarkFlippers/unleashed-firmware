#include "nice_o_raw.h"

#include <lib/subghz/subghz_file_encoder_worker.h>

/* Samples per tick. The stock Decode RAW scene uses the same figure: enough to get
 * through a long capture quickly, few enough that the UI still redraws between ticks. */
#define NICE_O_RAW_SAMPLES 400

/* Durations above this come from a gap the worker could not represent; feeding one to
 * the decoder only desynchronises it, so the tick ends there instead. */
#define NICE_O_RAW_MAX_DURATION 1000000UL

struct NiceORaw {
    SubGhzFileEncoderWorker* worker;
    SubGhzReceiver* receiver;
};

NiceORaw* nice_o_raw_alloc(void) {
    NiceORaw* instance = malloc(sizeof(NiceORaw));
    instance->worker = NULL;
    instance->receiver = NULL;
    return instance;
}

void nice_o_raw_free(NiceORaw* instance) {
    furi_assert(instance);
    nice_o_raw_stop(instance);
    free(instance);
}

bool nice_o_raw_start(NiceORaw* instance, const char* path, SubGhzReceiver* receiver) {
    furi_assert(instance);
    if(instance->worker) return false;

    instance->receiver = receiver;
    instance->worker = subghz_file_encoder_worker_alloc();

    /* NULL radio device: the samples are decoded, not transmitted, so the worker keeps
     * the long gaps between bursts instead of shortening them to fit a transmission. */
    if(!subghz_file_encoder_worker_start(instance->worker, path, NULL)) {
        subghz_file_encoder_worker_free(instance->worker);
        instance->worker = NULL;
        instance->receiver = NULL;
        return false;
    }

    furi_delay_ms(100); // the worker has to open and buffer the file before the first read
    return true;
}

void nice_o_raw_stop(NiceORaw* instance) {
    furi_assert(instance);
    if(!instance->worker) return;
    if(subghz_file_encoder_worker_is_running(instance->worker)) {
        subghz_file_encoder_worker_stop(instance->worker);
    }
    subghz_file_encoder_worker_free(instance->worker);
    instance->worker = NULL;
    instance->receiver = NULL;
}

bool nice_o_raw_next(NiceORaw* instance) {
    furi_assert(instance);
    if(!instance->worker) return false;

    for(uint32_t i = NICE_O_RAW_SAMPLES; i > 0; i--) {
        const LevelDuration ld = subghz_file_encoder_worker_get_level_duration(instance->worker);
        if(level_duration_is_reset(ld)) return false;
        if(level_duration_is_wait(ld)) return true;

        const uint32_t duration = level_duration_get_duration(ld);
        if(duration > NICE_O_RAW_MAX_DURATION) return true;
        subghz_receiver_decode(instance->receiver, level_duration_get_level(ld), duration);
    }
    return true;
}

void nice_o_raw_progress(NiceORaw* instance, FuriString* out) {
    furi_assert(instance);
    furi_string_reset(out);
    if(instance->worker) subghz_file_encoder_worker_get_text_progress(instance->worker, out);
}
