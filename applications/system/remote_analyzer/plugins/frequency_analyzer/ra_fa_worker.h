/**
 * @file ra_fa_worker.h
 * @brief Frequency analyzer scan worker. Internal CC1101 only.
 *
 * Ported from the Sub-GHz app's analyzer worker with the external-module paths removed: it
 * drives the internal radio's registers directly, and takes the frequency table and trigger
 * level as arguments instead of reaching into the Sub-GHz app's state.
 */
#pragma once

#include <lib/subghz/subghz_setting.h>

typedef struct RaFaWorker RaFaWorker;

typedef void (*RaFaWorkerPairCallback)(void* context, uint32_t frequency, float rssi, bool signal);

RaFaWorker* ra_fa_worker_alloc(SubGhzSetting* setting, float trigger_level);

void ra_fa_worker_free(RaFaWorker* instance);

void ra_fa_worker_set_pair_callback(
    RaFaWorker* instance,
    RaFaWorkerPairCallback callback,
    void* context);

void ra_fa_worker_start(RaFaWorker* instance);

void ra_fa_worker_stop(RaFaWorker* instance);

bool ra_fa_worker_is_running(RaFaWorker* instance);

void ra_fa_worker_set_trigger_level(RaFaWorker* instance, float value);

float ra_fa_worker_get_trigger_level(RaFaWorker* instance);

/** Snap a measured frequency to the nearest entry in the configured list. */
uint32_t ra_fa_worker_get_nearest_frequency(RaFaWorker* instance, uint32_t input);
