#include "ra_raw.h"

#include <lib/subghz/subghz_file_encoder_worker.h>
#include <lib/subghz/protocols/raw.h>
#include <lib/flipper_format/flipper_format.h>
#include <storage/storage.h>
#include <stdlib.h>

/* Samples per batch. The stock Decode RAW scene uses the same figure: enough to get through
 * a long capture quickly, few enough that the screen still redraws between ticks. */
#define RA_RAW_SAMPLES 400

/* A duration this long comes from a gap the worker could not represent. Feeding one to the
 * decoders only desynchronises them, so the batch ends there instead. */
#define RA_RAW_MAX_DURATION 1000000UL

struct RaRaw {
    SubGhzFileEncoderWorker* worker;
    SubGhzReceiver* receiver;
    FuriString* progress;
};

RaRaw* ra_raw_alloc(void) {
    RaRaw* instance = malloc(sizeof(RaRaw));
    instance->worker = NULL;
    instance->receiver = NULL;
    instance->progress = furi_string_alloc();
    return instance;
}

void ra_raw_free(RaRaw* instance) {
    furi_assert(instance);
    ra_raw_stop(instance);
    furi_string_free(instance->progress);
    free(instance);
}

bool ra_raw_start(RaRaw* instance, const char* path, SubGhzReceiver* receiver) {
    furi_assert(instance);
    if(instance->worker) return false;

    instance->receiver = receiver;
    instance->worker = subghz_file_encoder_worker_alloc();

    /* No radio device: the samples are decoded rather than transmitted, so the worker keeps
     * the long gaps between bursts instead of trimming them to fit a transmission. */
    if(!subghz_file_encoder_worker_start(instance->worker, path, NULL)) {
        subghz_file_encoder_worker_free(instance->worker);
        instance->worker = NULL;
        instance->receiver = NULL;
        return false;
    }

    furi_delay_ms(100); // the worker has to open and buffer the file before the first read
    return true;
}

void ra_raw_stop(RaRaw* instance) {
    furi_assert(instance);
    if(!instance->worker) return;
    if(subghz_file_encoder_worker_is_running(instance->worker)) {
        subghz_file_encoder_worker_stop(instance->worker);
    }
    subghz_file_encoder_worker_free(instance->worker);
    instance->worker = NULL;
    instance->receiver = NULL;
}

bool ra_raw_is_running(const RaRaw* instance) {
    furi_assert(instance);
    return instance->worker != NULL;
}

bool ra_raw_next(RaRaw* instance) {
    furi_assert(instance);
    if(!instance->worker) return false;

    for(uint32_t i = RA_RAW_SAMPLES; i > 0; i--) {
        const LevelDuration ld = subghz_file_encoder_worker_get_level_duration(instance->worker);
        if(level_duration_is_reset(ld)) return false;
        if(level_duration_is_wait(ld)) return true;

        const uint32_t duration = level_duration_get_duration(ld);
        if(duration > RA_RAW_MAX_DURATION) return true;
        subghz_receiver_decode(instance->receiver, level_duration_get_level(ld), duration);
    }
    return true;
}

uint8_t ra_raw_percent(RaRaw* instance) {
    furi_assert(instance);
    if(!instance->worker) return 0;

    // The worker reports it as "042%", so the leading number is the whole answer
    subghz_file_encoder_worker_get_text_progress(instance->worker, instance->progress);
    const int value = atoi(furi_string_get_cstr(instance->progress));
    if(value < 0) return 0;
    return (value > 100) ? 100 : (uint8_t)value;
}

bool ra_raw_file_is_raw(const char* path, uint32_t* frequency) {
    uint32_t hz = 0;
    bool is_raw = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff = flipper_format_file_alloc(storage);
    FuriString* protocol = furi_string_alloc();

    // Each read searches forwards from where the last one stopped, so both start from the top
    if(flipper_format_file_open_existing(fff, path)) {
        if(flipper_format_read_string(fff, "Protocol", protocol)) {
            is_raw = furi_string_equal_str(protocol, SUBGHZ_PROTOCOL_RAW_NAME);
        }
        flipper_format_rewind(fff);
        if(!flipper_format_read_uint32(fff, "Frequency", &hz, 1)) hz = 0;
    }

    furi_string_free(protocol);
    flipper_format_free(fff);
    furi_record_close(RECORD_STORAGE);

    if(frequency) *frequency = hz;
    return is_raw;
}
