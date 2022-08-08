#include "spectrum_analyzer.h"
#include "spectrum_analyzer_worker.h"

#include <furi_hal.h>
#include <furi.h>

#include <lib/drivers/cc1101_regs.h>

struct SpectrumAnalyzerWorker {
    FuriThread* thread;
    bool should_work;

    SpectrumAnalyzerWorkerCallback callback;
    void* callback_context;

    uint32_t channel0_frequency;
    uint32_t spacing;
    uint8_t width;
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
    furi_hal_subghz_load_registers((uint8_t*)filter_config);
}

static int32_t spectrum_analyzer_worker_thread(void* context) {
    furi_assert(context);
    SpectrumAnalyzerWorker* instance = context;

    FURI_LOG_D("SpectrumWorker", "spectrum_analyzer_worker_thread: Start");

    // Start CC1101
    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok650Async);
    furi_hal_subghz_set_frequency(433920000);
    furi_hal_subghz_flush_rx();
    furi_hal_subghz_rx();

    static const uint8_t radio_config[][2] = {
        {CC1101_FSCTRL1, 0x12},
        {CC1101_FSCTRL0, 0x00},

        {CC1101_AGCCTRL2, 0xC0},

        {CC1101_MDMCFG4, 0x6C},
        {CC1101_TEST2, 0x88},
        {CC1101_TEST1, 0x31},
        {CC1101_TEST0, 0x09},
        /* End  */
        {0, 0},
    };

    while(instance->should_work) {
        furi_delay_ms(50);

        // FURI_LOG_T("SpectrumWorker", "spectrum_analyzer_worker_thread: Worker Loop");
        furi_hal_subghz_idle();
        furi_hal_subghz_load_registers((uint8_t*)radio_config);

        // TODO: Check filter!
        // spectrum_analyzer_worker_set_filter(instance);

        instance->max_rssi_dec = 0;

        // Visit each channel non-consecutively
        for(uint8_t ch_offset = 0, chunk = 0; ch_offset < CHUNK_SIZE;
            ++chunk >= NUM_CHUNKS && ++ch_offset && (chunk = 0)) {
            uint8_t ch = chunk * CHUNK_SIZE + ch_offset;
            furi_hal_subghz_set_frequency(instance->channel0_frequency + (ch * instance->spacing));

            furi_hal_subghz_rx();
            furi_delay_ms(3);

            //         dec      dBm
            //max_ss = 127 ->  -10.5
            //max_ss = 0   ->  -74.0
            //max_ss = 255 ->  -74.5
            //max_ss = 128 -> -138.0
            instance->channel_ss[ch] = (furi_hal_subghz_get_rssi() + 138) * 2;

            if(instance->channel_ss[ch] > instance->max_rssi_dec) {
                instance->max_rssi_dec = instance->channel_ss[ch];
                instance->max_rssi = (instance->channel_ss[ch] / 2) - 138;
                instance->max_rssi_channel = ch;
            }

            furi_hal_subghz_idle();
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

    FURI_LOG_D("Spectrum", "spectrum_analyzer_worker_alloc: End");

    return instance;
}

void spectrum_analyzer_worker_free(SpectrumAnalyzerWorker* instance) {
    FURI_LOG_D("Spectrum", "spectrum_analyzer_worker_free");
    furi_assert(instance);
    furi_thread_free(instance->thread);
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
        "spectrum_analyzer_worker_set_frequencies - channel0_frequency= %u - spacing = %u - width = %u",
        channel0_frequency,
        spacing,
        width);

    instance->channel0_frequency = channel0_frequency;
    instance->spacing = spacing;
    instance->width = width;
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