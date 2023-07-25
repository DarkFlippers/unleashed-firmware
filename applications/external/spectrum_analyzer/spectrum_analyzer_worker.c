#include "spectrum_analyzer.h"
#include "spectrum_analyzer_worker.h"

#include <furi_hal.h>
#include <furi.h>

#include "helpers/radio_device_loader.h"

#include <lib/drivers/cc1101_regs.h>

struct SpectrumAnalyzerWorker {
    FuriThread* thread;
    bool should_work;

    SpectrumAnalyzerWorkerCallback callback;
    void* callback_context;

    const SubGhzDevice* radio_device;

    uint32_t channel0_frequency;
    uint32_t spacing;
    uint8_t width;
    uint8_t modulation;
    float max_rssi;
    uint8_t max_rssi_dec;
    uint8_t max_rssi_channel;

    uint8_t channel_ss[NUM_CHANNELS];
};

/* set the channel bandwidth */
void spectrum_analyzer_worker_set_filter(SpectrumAnalyzerWorker* instance) {
    uint8_t filter_config[2][2] = {
        {CC1101_MDMCFG4, 0},
        {0, 0},
    };

    // FURI_LOG_D("SpectrumWorker", "spectrum_analyzer_worker_set_filter: width = %u", instance->width);

    /* channel spacing should fit within 80% of channel filter bandwidth */
    switch(instance->width) {
    case NARROW:
        filter_config[0][1] = 0xFC; /* 39.2 kHz / .8 = 49 kHz --> 58 kHz */
        break;
    case ULTRAWIDE:
        filter_config[0][1] = 0x0C; /* 784 kHz / .8 = 980 kHz --> 812 kHz */
        break;
    default:
        filter_config[0][1] = 0x6C; /* 196 kHz / .8 = 245 kHz --> 270 kHz */
        break;
    }

    UNUSED(filter_config);
    // furi_hal_subghz_load_registers((uint8_t*)filter_config);
}

static int32_t spectrum_analyzer_worker_thread(void* context) {
    furi_assert(context);
    SpectrumAnalyzerWorker* instance = context;

    FURI_LOG_D("SpectrumWorker", "spectrum_analyzer_worker_thread: Start");

    // Start CC1101
    subghz_devices_reset(instance->radio_device);
    subghz_devices_load_preset(instance->radio_device, FuriHalSubGhzPresetOok650Async, NULL);
    subghz_devices_set_frequency(instance->radio_device, 433920000);
    subghz_devices_flush_rx(instance->radio_device);
    subghz_devices_set_rx(instance->radio_device);

    // Default modulation
    const uint8_t default_modulation[] = {

        /* Frequency Synthesizer Control */
        CC1101_FSCTRL0,
        0x00,
        CC1101_FSCTRL1,
        0x12, // IF = (26*10^6) / (2^10) * 0x12 = 304687.5 Hz

        // Modem Configuration
        // CC1101_MDMCFG0,
        // 0x00, // Channel spacing is 25kHz
        // CC1101_MDMCFG1,
        // 0x00, // Channel spacing is 25kHz
        // CC1101_MDMCFG2,
        // 0x30, // Format ASK/OOK, No preamble/sync
        // CC1101_MDMCFG3,
        // 0x32, // Data rate is 121.399 kBaud
        CC1101_MDMCFG4,
        0x6C, // Rx BW filter is 270.83 kHz

        /* Frequency Offset Compensation Configuration */
        // CC1101_FOCCFG,
        // 0x18, // no frequency offset compensation, POST_K same as PRE_K, PRE_K is 4K, GATE is off

        /* Automatic Gain Control */
        // CC1101_AGCCTRL0,
        // 0x91, // 10 - Medium hysteresis, medium asymmetric dead zone, medium gain ; 01 - 16 samples agc; 00 - Normal AGC, 01 - 8dB boundary
        // CC1101_AGCCTRL1,
        // 0x0, // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 0000 - RSSI to MAIN_TARGET
        CC1101_AGCCTRL2,
        0xC0, // 03 - The 3 highest DVGA gain settings can not be used; 000 - MAX LNA+LNA2; 000 - MAIN_TARGET 24 dB

        /* Frontend configuration */
        // CC1101_FREND0,
        // 0x11, // Adjusts current TX LO buffer + high is PATABLE[1]
        // CC1101_FREND1,
        // 0xB6, //

        CC1101_TEST2,
        0x88,
        CC1101_TEST1,
        0x31,
        CC1101_TEST0,
        0x09,

        /* End  */
        0,
        0,

        // ook_async_patable
        0x00,
        0xC0, // 12dBm 0xC0, 10dBm 0xC5, 7dBm 0xCD, 5dBm 0x86, 0dBm 0x50, -6dBm 0x37, -10dBm 0x26, -15dBm 0x1D, -20dBm 0x17, -30dBm 0x03
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00};

    // Narrow modulation
    const uint8_t narrow_modulation[] = {

        /* Frequency Synthesizer Control */
        CC1101_FSCTRL0,
        0x00,
        CC1101_FSCTRL1,
        0x00, // IF = (26*10^6) / (2^10) * 0x00 = 0 Hz

        // Modem Configuration
        // CC1101_MDMCFG0,
        // 0x00, // Channel spacing is 25kHz
        // CC1101_MDMCFG1,
        // 0x00, // Channel spacing is 25kHz
        // CC1101_MDMCFG2,
        // 0x30, // Format ASK/OOK, No preamble/sync
        // CC1101_MDMCFG3,
        // 0x32, // Data rate is 121.399 kBaud
        CC1101_MDMCFG4,
        0xFC, // Rx BW filter is 58.04 kHz

        /* Frequency Offset Compensation Configuration */
        // CC1101_FOCCFG,
        // 0x18, // no frequency offset compensation, POST_K same as PRE_K, PRE_K is 4K, GATE is off

        /* Automatic Gain Control */
        CC1101_AGCCTRL0,
        0x30, // 00 - NO hysteresis, symmetric dead zone, high gain ; 11 - 32 samples agc; 00 - Normal AGC, 00 - 8dB boundary
        CC1101_AGCCTRL1,
        0x0, // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 0000 - RSSI to MAIN_TARGET
        CC1101_AGCCTRL2,
        0x84, // 02 - The 2 highest DVGA gain settings can not be used; 000 - MAX LNA+LNA2; 100 - MAIN_TARGET 36 dB

        /* Frontend configuration */
        // CC1101_FREND0,
        // 0x11, // Adjusts current TX LO buffer + high is PATABLE[1]
        // CC1101_FREND1,
        // 0xB6, //

        CC1101_TEST2,
        0x88,
        CC1101_TEST1,
        0x31,
        CC1101_TEST0,
        0x09,

        /* End  */
        0,
        0,

        // ook_async_patable
        0x00,
        0xC0, // 12dBm 0xC0, 10dBm 0xC5, 7dBm 0xCD, 5dBm 0x86, 0dBm 0x50, -6dBm 0x37, -10dBm 0x26, -15dBm 0x1D, -20dBm 0x17, -30dBm 0x03
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00};

    const uint8_t* modulations[] = {default_modulation, narrow_modulation};

    while(instance->should_work) {
        furi_delay_ms(50);

        // FURI_LOG_T("SpectrumWorker", "spectrum_analyzer_worker_thread: Worker Loop");
        subghz_devices_idle(instance->radio_device);
        subghz_devices_load_preset(
            instance->radio_device,
            FuriHalSubGhzPresetCustom,
            (uint8_t*)modulations[instance->modulation]);
        //subghz_devices_load_preset(
        //    instance->radio_device, FuriHalSubGhzPresetCustom, (uint8_t*)default_modulation);
        //furi_hal_subghz_load_custom_preset(modulations[instance->modulation]);

        // TODO: Check filter!
        // spectrum_analyzer_worker_set_filter(instance);

        instance->max_rssi_dec = 0;

        // Visit each channel non-consecutively
        for(uint8_t ch_offset = 0, chunk = 0; ch_offset < CHUNK_SIZE;
            ++chunk >= NUM_CHUNKS && ++ch_offset && (chunk = 0)) {
            uint8_t ch = chunk * CHUNK_SIZE + ch_offset;

            if(subghz_devices_is_frequency_valid(
                   instance->radio_device,
                   instance->channel0_frequency + (ch * instance->spacing)))
                subghz_devices_set_frequency(
                    instance->radio_device,
                    instance->channel0_frequency + (ch * instance->spacing));

            subghz_devices_set_rx(instance->radio_device);
            furi_delay_ms(3);

            //         dec      dBm
            //max_ss = 127 ->  -10.5
            //max_ss = 0   ->  -74.0
            //max_ss = 255 ->  -74.5
            //max_ss = 128 -> -138.0
            instance->channel_ss[ch] = (subghz_devices_get_rssi(instance->radio_device) + 138) * 2;

            if(instance->channel_ss[ch] > instance->max_rssi_dec) {
                instance->max_rssi_dec = instance->channel_ss[ch];
                instance->max_rssi = (instance->channel_ss[ch] / 2) - 138;
                instance->max_rssi_channel = ch;
            }

            subghz_devices_idle(instance->radio_device);
        }

        // FURI_LOG_T("SpectrumWorker", "channel_ss[0]: %u", instance->channel_ss[0]);

        // Report results back to main thread
        if(instance->callback) {
            instance->callback(
                (void*)&(instance->channel_ss),
                instance->max_rssi,
                instance->max_rssi_dec,
                instance->max_rssi_channel,
                instance->callback_context);
        }
    }

    return 0;
}

SpectrumAnalyzerWorker* spectrum_analyzer_worker_alloc() {
    FURI_LOG_D("Spectrum", "spectrum_analyzer_worker_alloc: Start");

    SpectrumAnalyzerWorker* instance = malloc(sizeof(SpectrumAnalyzerWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "SpectrumWorker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, spectrum_analyzer_worker_thread);

    subghz_devices_init();

    instance->radio_device =
        radio_device_loader_set(instance->radio_device, SubGhzRadioDeviceTypeExternalCC1101);

    FURI_LOG_D("Spectrum", "spectrum_analyzer_worker_alloc: End");

    return instance;
}

void spectrum_analyzer_worker_free(SpectrumAnalyzerWorker* instance) {
    FURI_LOG_D("Spectrum", "spectrum_analyzer_worker_free");
    furi_assert(instance);
    furi_thread_free(instance->thread);

    subghz_devices_sleep(instance->radio_device);
    radio_device_loader_end(instance->radio_device);

    subghz_devices_deinit();

    free(instance);
}

void spectrum_analyzer_worker_set_callback(
    SpectrumAnalyzerWorker* instance,
    SpectrumAnalyzerWorkerCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->callback_context = context;
}

void spectrum_analyzer_worker_set_frequencies(
    SpectrumAnalyzerWorker* instance,
    uint32_t channel0_frequency,
    uint32_t spacing,
    uint8_t width) {
    furi_assert(instance);

    FURI_LOG_D(
        "SpectrumWorker",
        "spectrum_analyzer_worker_set_frequencies - channel0_frequency= %lu - spacing = %lu - width = %u",
        channel0_frequency,
        spacing,
        width);

    instance->channel0_frequency = channel0_frequency;
    instance->spacing = spacing;
    instance->width = width;
}

void spectrum_analyzer_worker_set_modulation(SpectrumAnalyzerWorker* instance, uint8_t modulation) {
    furi_assert(instance);

    FURI_LOG_D(
        "SpectrumWorker", "spectrum_analyzer_worker_set_modulation - modulation = %u", modulation);

    instance->modulation = modulation;
}

void spectrum_analyzer_worker_start(SpectrumAnalyzerWorker* instance) {
    FURI_LOG_D("Spectrum", "spectrum_analyzer_worker_start");

    furi_assert(instance);
    furi_assert(instance->should_work == false);

    instance->should_work = true;
    furi_thread_start(instance->thread);
}

void spectrum_analyzer_worker_stop(SpectrumAnalyzerWorker* instance) {
    FURI_LOG_D("Spectrum", "spectrum_analyzer_worker_stop");
    furi_assert(instance);
    furi_assert(instance->should_work == true);

    instance->should_work = false;
    furi_thread_join(instance->thread);
}