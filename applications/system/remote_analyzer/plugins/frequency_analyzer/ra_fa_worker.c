#include "ra_fa_worker.h"

#include <lib/drivers/cc1101.h>

#include <furi.h>
#include <furi_hal.h>
#include <float_tools.h>

#define TAG "RaFaWorker"

static const uint8_t ra_fa_preset_ook_58khz[][2] = {
    {CC1101_MDMCFG4, 0b11110111}, // Rx BW filter is 58.035714kHz
    {0, 0},
};

static const uint8_t ra_fa_preset_ook_650khz[][2] = {
    {CC1101_MDMCFG4, 0b00010111}, // Rx BW filter is 650.000kHz
    {0, 0},
};

struct RaFaWorker {
    FuriThread* thread;

    volatile bool worker_running;
    uint8_t sample_hold_counter;
    SubGhzSetting* setting;

    float filVal;
    float trigger_level;

    RaFaWorkerPairCallback pair_callback;
    void* context;
};

typedef struct {
    uint32_t frequency_coarse;
    float rssi_coarse;
    uint32_t frequency_fine;
    float rssi_fine;
} RaFaFrequencyRSSI;

static void ra_fa_worker_load_registers(const uint8_t data[][2]) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    size_t i = 0;
    while(data[i][0]) {
        cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, data[i][0], data[i][1]);
        i++;
    }
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

// running average with adaptive coefficient
static uint32_t ra_fa_worker_running_average_adaptive(RaFaWorker* instance, uint32_t newVal) {
    float k;
    float newValFloat = newVal;
    // the sharpness of the filter depends on the absolute value of the difference
    if(fabsf(newValFloat - instance->filVal) > 500000.f)
        k = 0.9f;
    else
        k = 0.03f;

    instance->filVal += (newValFloat - instance->filVal) * k;
    return (uint32_t)instance->filVal;
}

/** Park the chip on one frequency and read its RSSI. */
static float ra_fa_worker_measure(uint32_t frequency, uint32_t* tuned) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_switch_to_idle(&furi_hal_spi_bus_handle_subghz);
    *tuned = cc1101_set_frequency(&furi_hal_spi_bus_handle_subghz, frequency);
    cc1101_calibrate(&furi_hal_spi_bus_handle_subghz);
    furi_check(cc1101_wait_status_state(&furi_hal_spi_bus_handle_subghz, CC1101StateIDLE, 10000));
    cc1101_switch_to_rx(&furi_hal_spi_bus_handle_subghz);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);

    furi_delay_ms(2);
    return furi_hal_subghz_get_rssi();
}

static int32_t ra_fa_worker_thread(void* context) {
    RaFaWorker* instance = context;

    RaFaFrequencyRSSI frequency_rssi = {
        .frequency_coarse = 0, .rssi_coarse = 0, .frequency_fine = 0, .rssi_fine = 0};
    float rssi = 0;
    uint32_t frequency = 0;
    float rssi_temp = 0;
    uint32_t frequency_temp = 0;

    furi_hal_subghz_reset();

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_flush_rx(&furi_hal_spi_bus_handle_subghz);
    cc1101_flush_tx(&furi_hal_spi_bus_handle_subghz);
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG0, CC1101IocfgHW);
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_MDMCFG3, 0b01111111); // symbol rate
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
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);

    while(instance->worker_running) {
        furi_delay_ms(10);

        frequency_rssi.rssi_coarse = -127.0f;
        frequency_rssi.rssi_fine = -127.0f;
        furi_hal_subghz_idle();
        ra_fa_worker_load_registers(ra_fa_preset_ook_650khz);

        // First stage: coarse scan over the configured frequency list
        for(size_t i = 0; i < subghz_setting_get_frequency_count(instance->setting); i++) {
            uint32_t current_frequency = subghz_setting_get_frequency(instance->setting, i);
            // The three 46x bands and anything above 920MHz read as noise on this radio
            if(furi_hal_subghz_is_frequency_valid(current_frequency) &&
               current_frequency != 462750000 && current_frequency != 467750000 &&
               current_frequency != 464000000 && current_frequency <= 920000000) {
                rssi = ra_fa_worker_measure(current_frequency, &frequency);

                if(frequency_rssi.rssi_coarse < rssi) {
                    frequency_rssi.rssi_coarse = rssi;
                    frequency_rssi.frequency_coarse = frequency;
                }
            }
        }

        // Second stage: fine scan around the winner, -0.3 ... +0.3MHz in 20kHz steps
        if(frequency_rssi.rssi_coarse > instance->trigger_level) {
            furi_hal_subghz_idle();
            ra_fa_worker_load_registers(ra_fa_preset_ook_58khz);
            for(uint32_t i = frequency_rssi.frequency_coarse - 300000;
                i < frequency_rssi.frequency_coarse + 300000;
                i += 20000) {
                if(furi_hal_subghz_is_frequency_valid(i)) {
                    rssi = ra_fa_worker_measure(i, &frequency);
                    if(frequency_rssi.rssi_fine < rssi) {
                        frequency_rssi.rssi_fine = rssi;
                        frequency_rssi.frequency_fine = frequency;
                    }
                }
            }
        }

        if(frequency_rssi.rssi_fine > instance->trigger_level) {
            instance->sample_hold_counter = 20;
            rssi_temp = frequency_rssi.rssi_fine;
            frequency_temp = frequency_rssi.frequency_fine;

            if(!float_is_equal(instance->filVal, 0.f)) {
                frequency_rssi.frequency_fine =
                    ra_fa_worker_running_average_adaptive(instance, frequency_rssi.frequency_fine);
            }
            if(instance->pair_callback) {
                instance->pair_callback(
                    instance->context,
                    frequency_rssi.frequency_fine,
                    frequency_rssi.rssi_fine,
                    true);
            }
        } else if(
            (frequency_rssi.rssi_coarse > instance->trigger_level) &&
            (instance->sample_hold_counter < 10)) {
            instance->sample_hold_counter = 20;
            rssi_temp = frequency_rssi.rssi_coarse;
            frequency_temp = frequency_rssi.frequency_coarse;

            if(!float_is_equal(instance->filVal, 0.f)) {
                frequency_rssi.frequency_coarse = ra_fa_worker_running_average_adaptive(
                    instance, frequency_rssi.frequency_coarse);
            }
            if(instance->pair_callback) {
                instance->pair_callback(
                    instance->context,
                    frequency_rssi.frequency_coarse,
                    frequency_rssi.rssi_coarse,
                    true);
            }
        } else {
            if(instance->sample_hold_counter > 0) {
                instance->sample_hold_counter--;
                if(instance->sample_hold_counter == 18 && instance->pair_callback) {
                    instance->pair_callback(instance->context, frequency_temp, rssi_temp, false);
                }
            } else {
                instance->filVal = 0;
                if(instance->pair_callback) {
                    instance->pair_callback(instance->context, 0, 0, false);
                }
            }
        }
    }

    furi_hal_subghz_idle();
    /* Reset drops the custom AGC/BW config above; SRES also reverts IOCFG2, so re-park the
     * RF switch - this leaves the radio in the same state as after boot, which is what the
     * receiver expects to find when the app goes back to Read. */
    furi_hal_subghz_reset();
    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();

    return 0;
}

RaFaWorker* ra_fa_worker_alloc(SubGhzSetting* setting, float trigger_level) {
    furi_assert(setting);
    RaFaWorker* instance = malloc(sizeof(RaFaWorker));

    instance->thread = furi_thread_alloc_ex("RaFaWorker", 2048, ra_fa_worker_thread, instance);
    instance->worker_running = false;
    instance->sample_hold_counter = 0;
    instance->setting = setting;
    instance->filVal = 0;
    instance->trigger_level = trigger_level;
    instance->pair_callback = NULL;
    instance->context = NULL;

    return instance;
}

void ra_fa_worker_free(RaFaWorker* instance) {
    furi_assert(instance);
    furi_thread_free(instance->thread);
    free(instance);
}

void ra_fa_worker_set_pair_callback(
    RaFaWorker* instance,
    RaFaWorkerPairCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(context);
    instance->pair_callback = callback;
    instance->context = context;
}

void ra_fa_worker_start(RaFaWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);

    instance->worker_running = true;
    furi_thread_start(instance->thread);
}

void ra_fa_worker_stop(RaFaWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->worker_running);

    instance->worker_running = false;
    furi_thread_join(instance->thread);
}

bool ra_fa_worker_is_running(RaFaWorker* instance) {
    furi_assert(instance);
    return instance->worker_running;
}

void ra_fa_worker_set_trigger_level(RaFaWorker* instance, float value) {
    furi_assert(instance);
    instance->trigger_level = value;
}

float ra_fa_worker_get_trigger_level(RaFaWorker* instance) {
    furi_assert(instance);
    return instance->trigger_level;
}

uint32_t ra_fa_worker_get_nearest_frequency(RaFaWorker* instance, uint32_t input) {
    uint32_t prev_freq = 0;
    uint32_t result = 0;
    uint32_t current;

    for(size_t i = 0; i < subghz_setting_get_frequency_count(instance->setting); i++) {
        current = subghz_setting_get_frequency(instance->setting, i);
        if(current == 0) continue;
        if(current == input) {
            result = current;
            break;
        }
        if(current > input && prev_freq < input) {
            result = (current - input < input - prev_freq) ? current : prev_freq;
            break;
        }
        prev_freq = current;
    }

    return result;
}
