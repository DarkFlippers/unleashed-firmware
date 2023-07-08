#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef void (*MusicWorkerCallback)(
    uint8_t semitone,
    uint8_t dots,
    uint8_t duration,
    float position,
    void* context);

typedef struct MusicWorker MusicWorker;

MusicWorker* music_worker_alloc();

void music_worker_clear(MusicWorker* instance);

void music_worker_free(MusicWorker* instance);

bool music_worker_load(MusicWorker* instance, const char* file_path);

bool music_worker_load_fmf_from_file(MusicWorker* instance, const char* file_path);

bool music_worker_load_rtttl_from_file(MusicWorker* instance, const char* file_path);

bool music_worker_load_rtttl_from_string(MusicWorker* instance, const char* string);

void music_worker_set_callback(MusicWorker* instance, MusicWorkerCallback callback, void* context);

void music_worker_set_volume(MusicWorker* instance, float volume);

void music_worker_start(MusicWorker* instance);

void music_worker_stop(MusicWorker* instance);

bool music_worker_is_playing(MusicWorker* instance);
