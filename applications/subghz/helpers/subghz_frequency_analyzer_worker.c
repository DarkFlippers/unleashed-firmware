#include "subghz_frequency_analyzer_worker.h"
#include <lib/drivers/cc1101.h>

#include <furi.h>

#include "../subghz_i.h"

<<<<<<< HEAD
#define SUBGHZ_FREQUENCY_ANALYZER_THRESHOLD -90.0f
=======
#define TAG "SubghzFrequencyAnalyzerWorker"

#define SUBGHZ_FREQUENCY_ANALYZER_THRESHOLD -95.0f
>>>>>>> upstream/dev

static const uint8_t subghz_preset_ook_58khz[][2] = {
    {CC1101_MDMCFG4, 0b11110111}, // Rx BW filter is 58.035714kHz
    /* End  */
    {0, 0},
};

static const uint8_t subghz_preset_ook_650khz[][2] = {
    {CC1101_MDMCFG4, 0b00010111}, // Rx BW filter is 650.000kHz
    /* End  */
    {0, 0},
};

struct SubGhzFrequencyAnalyzerWorker {
    FuriThread* thread;

    volatile bool worker_running;
    uint8_t sample_hold_counter;
    FrequencyRSSI frequency_rssi_buf;
    SubGhzSetting* setting;

    float filVal;

    SubGhzFrequencyAnalyzerWorkerPairCallback pair_callback;
    void* context;
};

static void subghz_frequency_analyzer_worker_load_registers(const uint8_t data[][2]) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
<<<<<<< HEAD
    uint32_t i = 0;
=======
    size_t i = 0;
>>>>>>> upstream/dev
    while(data[i][0]) {
        cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, data[i][0], data[i][1]);
        i++;
    }
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

// running average with adaptive coefficient
static uint32_t subghz_frequency_analyzer_worker_expRunningAverageAdaptive(
    SubGhzFrequencyAnalyzerWorker* instance,
    uint32_t newVal) {
    float k;
    float newValFloat = newVal;
    // the sharpness of the filter depends on the absolute value of the difference
    if(fabs(newValFloat - instance->filVal) > 500000)
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

    FrequencyRSSI frequency_rssi = {.frequency = 0, .rssi = 0};
<<<<<<< HEAD
    float rssi;
    uint32_t frequency;
    uint32_t frequency_start;
=======
    float rssi = 0;
    uint32_t frequency = 0;
>>>>>>> upstream/dev
    CC1101Status status;

    //Start CC1101
    furi_hal_subghz_reset();

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_flush_rx(&furi_hal_spi_bus_handle_subghz);
    cc1101_flush_tx(&furi_hal_spi_bus_handle_subghz);
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG0, CC1101IocfgHW);
<<<<<<< HEAD
    cc1101_write_reg(
        &furi_hal_spi_bus_handle_subghz,
        CC1101_AGCCTRL2,
        0b0000111); // 00 - DVGA all; 000 - MAX LNA+LNA2; 111 - MAIN_TARGET 42 dB
    cc1101_write_reg(
        &furi_hal_spi_bus_handle_subghz,
        CC1101_AGCCTRL1,
        0b00000000); // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 0000 - RSSI to MAIN_TARGET
    cc1101_write_reg(
        &furi_hal_spi_bus_handle_subghz,
        CC1101_AGCCTRL0,
        0b00000001); // 00 - No hysteresis, medium asymmetric dead zone, medium gain ; 00 - 8 samples agc; 00 - Normal AGC, 01 - 8dB boundary
=======
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_MDMCFG3,
                     0b11111111); // symbol rate
    cc1101_write_reg(
        &furi_hal_spi_bus_handle_subghz,
        CC1101_AGCCTRL2,
        0b00000111); // 00 - DVGA all; 000 - MAX LNA+LNA2; 111 - MAGN_TARGET 42 dB
    cc1101_write_reg(
        &furi_hal_spi_bus_handle_subghz,
        CC1101_AGCCTRL1,
        0b00001000); // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 1000 - Absolute carrier sense threshold disabled
    cc1101_write_reg(
        &furi_hal_spi_bus_handle_subghz,
        CC1101_AGCCTRL0,
        0b00110000); // 00 - No hysteresis, medium asymmetric dead zone, medium gain ; 11 - 64 samples agc; 00 - Normal AGC, 00 - 4dB boundary
>>>>>>> upstream/dev

    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);

    while(instance->worker_running) {
        osDelay(10);

        float rssi_min = 26.0f;
        float rssi_avg = 0;
        size_t rssi_avg_samples = 0;

        frequency_rssi.rssi = -127.0f;
        furi_hal_subghz_idle();
        subghz_frequency_analyzer_worker_load_registers(subghz_preset_ook_650khz);

<<<<<<< HEAD
=======
        // First stage: coarse scan
>>>>>>> upstream/dev
        for(size_t i = 0; i < subghz_setting_get_frequency_count(instance->setting); i++) {
            if(furi_hal_subghz_is_frequency_valid(
                   subghz_setting_get_frequency(instance->setting, i))) {
                furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
                cc1101_switch_to_idle(&furi_hal_spi_bus_handle_subghz);
                frequency = cc1101_set_frequency(
                    &furi_hal_spi_bus_handle_subghz,
                    subghz_setting_get_frequency(instance->setting, i));

                cc1101_calibrate(&furi_hal_spi_bus_handle_subghz);
                do {
                    status = cc1101_get_status(&furi_hal_spi_bus_handle_subghz);
                } while(status.STATE != CC1101StateIDLE);

                cc1101_switch_to_rx(&furi_hal_spi_bus_handle_subghz);
                furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);

<<<<<<< HEAD
                osDelay(3);
=======
                // delay will be in range between 1 and 2ms
                osDelay(2);

>>>>>>> upstream/dev
                rssi = furi_hal_subghz_get_rssi();

                rssi_avg += rssi;
                rssi_avg_samples++;

                if(rssi < rssi_min) rssi_min = rssi;

                if(frequency_rssi.rssi < rssi) {
                    frequency_rssi.rssi = rssi;
                    frequency_rssi.frequency = frequency;
                }
            }
        }

<<<<<<< HEAD
        if(frequency_rssi.rssi > SUBGHZ_FREQUENCY_ANALYZER_THRESHOLD) {
            //  -0.5 ... 433.92 ... +0.5
            frequency_start = frequency_rssi.frequency - 500000;
            frequency_rssi.rssi = -127.0;
            furi_hal_subghz_idle();
            subghz_frequency_analyzer_worker_load_registers(subghz_preset_ook_58khz);
            //step 10KHz
            for(uint32_t i = frequency_start; i < frequency_start + 500000; i += 10000) {
=======
        FURI_LOG_T(
            TAG,
            "RSSI: avg %f, max %f at %u, min %f",
            (double)(rssi_avg / rssi_avg_samples),
            (double)frequency_rssi.rssi,
            frequency_rssi.frequency,
            (double)rssi_min);

        // Second stage: fine scan
        if(frequency_rssi.rssi > SUBGHZ_FREQUENCY_ANALYZER_THRESHOLD) {
            FURI_LOG_D(TAG, "~:%u:%f", frequency_rssi.frequency, (double)frequency_rssi.rssi);

            frequency_rssi.rssi = -127.0;
            furi_hal_subghz_idle();
            subghz_frequency_analyzer_worker_load_registers(subghz_preset_ook_58khz);
            //-0.3 ... 433.92 ... +0.3 step 10KHz
            for(uint32_t i = frequency_rssi.frequency - 300000;
                i < frequency_rssi.frequency + 300000;
                i += 20000) {
>>>>>>> upstream/dev
                if(furi_hal_subghz_is_frequency_valid(i)) {
                    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
                    cc1101_switch_to_idle(&furi_hal_spi_bus_handle_subghz);
                    frequency = cc1101_set_frequency(&furi_hal_spi_bus_handle_subghz, i);

                    cc1101_calibrate(&furi_hal_spi_bus_handle_subghz);
                    do {
                        status = cc1101_get_status(&furi_hal_spi_bus_handle_subghz);
                    } while(status.STATE != CC1101StateIDLE);

                    cc1101_switch_to_rx(&furi_hal_spi_bus_handle_subghz);
                    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);

<<<<<<< HEAD
                    osDelay(5);
=======
                    // delay will be in range between 1 and 2ms
                    osDelay(2);

>>>>>>> upstream/dev
                    rssi = furi_hal_subghz_get_rssi();
                    if(frequency_rssi.rssi < rssi) {
                        frequency_rssi.rssi = rssi;
                        frequency_rssi.frequency = frequency;
                    }
                }
            }
        }

<<<<<<< HEAD
        if(frequency_rssi.rssi > SUBGHZ_FREQUENCY_ANALYZER_THRESHOLD) {
            instance->count_repet = 20;
=======
        // Deliver results
        if(frequency_rssi.rssi > SUBGHZ_FREQUENCY_ANALYZER_THRESHOLD) {
            FURI_LOG_D(TAG, "=:%u:%f", frequency_rssi.frequency, (double)frequency_rssi.rssi);

            instance->sample_hold_counter = 20;
>>>>>>> upstream/dev
            if(instance->filVal) {
                frequency_rssi.frequency =
                    subghz_frequency_analyzer_worker_expRunningAverageAdaptive(
                        instance, frequency_rssi.frequency);
            }
            // Deliver callback
            if(instance->pair_callback) {
                instance->pair_callback(
                    instance->context, frequency_rssi.frequency, frequency_rssi.rssi);
            }
        } else {
            if(instance->sample_hold_counter > 0) {
                instance->sample_hold_counter--;
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
    SubGhzFrequencyAnalyzerWorker* instance = malloc(sizeof(SubGhzFrequencyAnalyzerWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "SubGhzFAWorker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subghz_frequency_analyzer_worker_thread);

    instance->setting = subghz_setting_alloc();
    subghz_setting_load(instance->setting, "/ext/subghz/assets/setting_frequency_analyzer_user");
    return instance;
}

void subghz_frequency_analyzer_worker_free(SubGhzFrequencyAnalyzerWorker* instance) {
    furi_assert(instance);

    furi_thread_free(instance->thread);
    subghz_setting_free(instance->setting);
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
