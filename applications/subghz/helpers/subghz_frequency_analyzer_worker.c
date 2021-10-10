#include "subghz_frequency_analyzer_worker.h"

#include <furi.h>

#include "../subghz_i.h"

struct SubGhzFrequencyAnalyzerWorker {
    FuriThread* thread;

    volatile bool worker_running;
    uint8_t count_repet;
    FrequencyRSSI frequency_rssi_buf;

    float filVal;

    SubGhzFrequencyAnalyzerWorkerPairCallback pair_callback;
    void* context;
};

// running average with adaptive coefficient
static uint32_t subghz_frequency_analyzer_worker_expRunningAverageAdaptive(
    SubGhzFrequencyAnalyzerWorker* instance,
    uint32_t newVal) {
    float k;
    float newValFloat = newVal;
    // the sharpness of the filter depends on the absolute value of the difference
    if(abs(newValFloat - instance->filVal) > 500000)
        k = 0.9;
    else
        k = 0.03;

    instance->filVal += (newValFloat - instance->filVal) * k;
    return (uint32_t)instance->filVal;
}

/** Worker thread
 * 
 * @param context 
 * @return exit code 
 */
static int32_t subghz_frequency_analyzer_worker_thread(void* context) {
    SubGhzFrequencyAnalyzerWorker* instance = context;

    FrequencyRSSI frequency_rssi;
    float rssi;
    uint32_t frequency;
    uint32_t frequency_start;

    //Start CC1101
    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok650Async);
    furi_hal_subghz_set_frequency(433920000);
    furi_hal_subghz_flush_rx();
    furi_hal_subghz_rx();

    while(instance->worker_running) {
        osDelay(10);
        frequency_rssi.rssi = -127.0f;
        for(size_t i = 0; i < subghz_frequencies_count; i++) {
            if(furi_hal_subghz_is_frequency_valid(subghz_frequencies[i])) {
                furi_hal_subghz_idle();
                frequency = furi_hal_subghz_set_frequency(subghz_frequencies[i]);
                furi_hal_subghz_rx();
                osDelay(3);
                rssi = furi_hal_subghz_get_rssi();
                if(frequency_rssi.rssi < rssi) {
                    frequency_rssi.rssi = rssi;
                    frequency_rssi.frequency = frequency;
                }
            }
        }

        if(frequency_rssi.rssi > -90.0) {
            //  -0.5 ... 433.92 ... +0.5
            frequency_start = frequency_rssi.frequency - 250000;
            //step 10KHz
            for(uint32_t i = frequency_start; i < frequency_start + 500000; i += 10000) {
                if(furi_hal_subghz_is_frequency_valid(i)) {
                    furi_hal_subghz_idle();
                    frequency = furi_hal_subghz_set_frequency(i);
                    furi_hal_subghz_rx();
                    osDelay(3);
                    rssi = furi_hal_subghz_get_rssi();
                    if(frequency_rssi.rssi < rssi) {
                        frequency_rssi.rssi = rssi;
                        frequency_rssi.frequency = frequency;
                    }
                }
            }
        }

        if(frequency_rssi.rssi > -90.0) {
            instance->count_repet = 20;
            if(instance->filVal) {
                frequency_rssi.frequency =
                    subghz_frequency_analyzer_worker_expRunningAverageAdaptive(
                        instance, frequency_rssi.frequency);
            }
            if(instance->pair_callback)
                instance->pair_callback(
                    instance->context, frequency_rssi.frequency, frequency_rssi.rssi);

        } else {
            if(instance->count_repet > 0) {
                instance->count_repet--;
            } else {
                instance->filVal = 0;
                if(instance->pair_callback) instance->pair_callback(instance->context, 0, 0);
            }
        }
    }

    //Stop CC1101
    furi_hal_subghz_idle();
    furi_hal_subghz_sleep();

    return 0;
}

SubGhzFrequencyAnalyzerWorker* subghz_frequency_analyzer_worker_alloc() {
    SubGhzFrequencyAnalyzerWorker* instance = furi_alloc(sizeof(SubGhzFrequencyAnalyzerWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "subghz_frequency_analyzer_worker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subghz_frequency_analyzer_worker_thread);

    return instance;
}

void subghz_frequency_analyzer_worker_free(SubGhzFrequencyAnalyzerWorker* instance) {
    furi_assert(instance);

    furi_thread_free(instance->thread);

    free(instance);
}

void subghz_frequency_analyzer_worker_set_pair_callback(
    SubGhzFrequencyAnalyzerWorker* instance,
    SubGhzFrequencyAnalyzerWorkerPairCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(context);
    instance->pair_callback = callback;
    instance->context = context;
}

void subghz_frequency_analyzer_worker_start(SubGhzFrequencyAnalyzerWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);

    instance->worker_running = true;

    furi_thread_start(instance->thread);
}

void subghz_frequency_analyzer_worker_stop(SubGhzFrequencyAnalyzerWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->worker_running);

    instance->worker_running = false;

    furi_thread_join(instance->thread);
}

bool subghz_frequency_analyzer_worker_is_running(SubGhzFrequencyAnalyzerWorker* instance) {
    furi_assert(instance);
    return instance->worker_running;
}
