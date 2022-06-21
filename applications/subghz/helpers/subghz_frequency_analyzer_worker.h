#pragma once

#include <furi_hal.h>
#include "../subghz_i.h"

typedef struct SubGhzFrequencyAnalyzerWorker SubGhzFrequencyAnalyzerWorker;

typedef void (
    *SubGhzFrequencyAnalyzerWorkerPairCallback)(void* context, uint32_t frequency, float rssi);

typedef struct {
    uint32_t frequency_coarse;
    float rssi_coarse;
    uint32_t frequency_fine;
    float rssi_fine;
} FrequencyRSSI;

/** Allocate SubGhzFrequencyAnalyzerWorker
 * 
 * @param context SubGhz* context
 * @return SubGhzFrequencyAnalyzerWorker* 
 */
SubGhzFrequencyAnalyzerWorker* subghz_frequency_analyzer_worker_alloc(void* context);

/** Free SubGhzFrequencyAnalyzerWorker
 * 
 * @param instance SubGhzFrequencyAnalyzerWorker instance
 */
void subghz_frequency_analyzer_worker_free(SubGhzFrequencyAnalyzerWorker* instance);

/** Pair callback SubGhzFrequencyAnalyzerWorker
 * 
 * @param instance SubGhzFrequencyAnalyzerWorker instance
 * @param callback SubGhzFrequencyAnalyzerWorkerOverrunCallback callback
 * @param context 
 */
void subghz_frequency_analyzer_worker_set_pair_callback(
    SubGhzFrequencyAnalyzerWorker* instance,
    SubGhzFrequencyAnalyzerWorkerPairCallback callback,
    void* context);

/** Start SubGhzFrequencyAnalyzerWorker
 * 
 * @param instance SubGhzFrequencyAnalyzerWorker instance
 */
void subghz_frequency_analyzer_worker_start(SubGhzFrequencyAnalyzerWorker* instance);

/** Stop SubGhzFrequencyAnalyzerWorker
 * 
 * @param instance SubGhzFrequencyAnalyzerWorker instance
 */
void subghz_frequency_analyzer_worker_stop(SubGhzFrequencyAnalyzerWorker* instance);

/** Check if worker is running
 * @param instance SubGhzFrequencyAnalyzerWorker instance
 * @return bool - true if running
 */
bool subghz_frequency_analyzer_worker_is_running(SubGhzFrequencyAnalyzerWorker* instance);
