#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef void (*MusicPlayerWorkerCallback)(
    uint8_t semitone,
    uint8_t dots,
    uint8_t duration,
    float position,
    void* context);

typedef struct MusicPlayerWorker MusicPlayerWorker;

MusicPlayerWorker* music_player_worker_alloc();

void music_player_worker_free(MusicPlayerWorker* instance);

bool music_player_worker_load(MusicPlayerWorker* instance, const char* file_path);

bool music_player_worker_load_fmf_from_file(MusicPlayerWorker* instance, const char* file_path);

bool music_player_worker_load_rtttl_from_file(MusicPlayerWorker* instance, const char* file_path);

bool music_player_worker_load_rtttl_from_string(MusicPlayerWorker* instance, const char* string);

void music_player_worker_set_callback(
    MusicPlayerWorker* instance,
    MusicPlayerWorkerCallback callback,
    void* context);

void music_player_worker_set_volume(MusicPlayerWorker* instance, float volume);

void music_player_worker_start(MusicPlayerWorker* instance);

void music_player_worker_stop(MusicPlayerWorker* instance);
