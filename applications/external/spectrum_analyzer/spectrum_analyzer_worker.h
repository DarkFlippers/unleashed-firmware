#pragma once

#include <stdint.h>

typedef void (*SpectrumAnalyzerWorkerCallback)(
    void* chan_table,
    float max_rssi,
    uint8_t max_rssi_dec,
    uint8_t max_rssi_channel,
    void* context);

typedef struct SpectrumAnalyzerWorker SpectrumAnalyzerWorker;

SpectrumAnalyzerWorker* spectrum_analyzer_worker_alloc();

void spectrum_analyzer_worker_free(SpectrumAnalyzerWorker* instance);

void spectrum_analyzer_worker_set_callback(
    SpectrumAnalyzerWorker* instance,
    SpectrumAnalyzerWorkerCallback callback,
    void* context);

void spectrum_analyzer_worker_set_filter(SpectrumAnalyzerWorker* instance);

void spectrum_analyzer_worker_set_frequencies(
    SpectrumAnalyzerWorker* instance,
    uint32_t channel0_frequency,
    uint32_t spacing,
    uint8_t width);

void spectrum_analyzer_worker_set_modulation(SpectrumAnalyzerWorker* instance, uint8_t modulation);

void spectrum_analyzer_worker_start(SpectrumAnalyzerWorker* instance);

void spectrum_analyzer_worker_stop(SpectrumAnalyzerWorker* instance);
