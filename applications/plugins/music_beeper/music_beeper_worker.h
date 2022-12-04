#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*MusicBeeperWorkerCallback)(
    uint8_t semitone,
    uint8_t dots,
    uint8_t duration,
    float position,
    void* context);

typedef struct MusicBeeperWorker MusicBeeperWorker;

MusicBeeperWorker* music_beeper_worker_alloc();

void music_beeper_worker_clear(MusicBeeperWorker* instance);

void music_beeper_worker_free(MusicBeeperWorker* instance);

bool music_beeper_worker_load(MusicBeeperWorker* instance, const char* file_path);

bool music_beeper_worker_load_fmf_from_file(MusicBeeperWorker* instance, const char* file_path);

bool music_beeper_worker_load_rtttl_from_file(MusicBeeperWorker* instance, const char* file_path);

bool music_beeper_worker_load_rtttl_from_string(MusicBeeperWorker* instance, const char* string);

void music_beeper_worker_set_callback(
    MusicBeeperWorker* instance,
    MusicBeeperWorkerCallback callback,
    void* context);

void music_beeper_worker_set_volume(MusicBeeperWorker* instance, float volume);

void music_beeper_worker_start(MusicBeeperWorker* instance);

void music_beeper_worker_stop(MusicBeeperWorker* instance);

#ifdef __cplusplus
}
#endif
