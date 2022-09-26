#include "subbrute_worker.h"

#include <subghz/environment.h>
#include <subghz/transmitter.h>
#include <flipper_format_i.h>
#include <lib/subghz/subghz_tx_rx_worker.h>

#define TAG "SubBruteWorker"

struct SubBruteWorker {
    SubGhzTxRxWorker* subghz_txrx;
    volatile bool worker_running;
    volatile bool worker_manual_mode;
    bool is_manual_init;
    bool is_continuous_worker;

    SubGhzEnvironment* environment;
    SubGhzTransmitter* transmitter;
    FlipperFormat* flipper_format;

    uint32_t last_time_tx_data;

    // Preset and frequency needed
    FuriHalSubGhzPreset preset;
    uint32_t frequency;
    string_t protocol_name;

    //SubBruteWorkerCallback callback;
    //void* context;
};

/** Taken from subghz_tx_rx_worker.c */
#define SUBBRUTE_TXRX_WORKER_BUF_SIZE 2048
#define SUBBRUTE_TXRX_WORKER_MAX_TXRX_SIZE 60
#define SUBBRUTE_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF 40
#define SUBBRUTE_TX_TIMEOUT 5
#define SUBBRUTE_SEND_DELAY 20

SubBruteWorker* subbrute_worker_alloc() {
    SubBruteWorker* instance = malloc(sizeof(SubBruteWorker));

    //instance->status = SubBruteWorkerStatusIDLE;
    instance->worker_running = false;
    instance->worker_manual_mode = false;

    //instance->environment = subghz_environment_alloc();
    instance->transmitter = NULL;

    instance->flipper_format = flipper_format_string_alloc();
    string_init(instance->protocol_name);

    // SubGhzTxRxWorker
    instance->subghz_txrx = subghz_tx_rx_worker_alloc();

    return instance;
}

void subbrute_worker_free(SubBruteWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);

    if(instance->transmitter != NULL) {
        subghz_transmitter_free(instance->transmitter);
        instance->transmitter = NULL;
    }

    /*if(instance->environment != NULL) {
        subghz_environment_free(instance->environment);
        instance->environment = NULL;
    }*/

    flipper_format_free(instance->flipper_format);

    string_clear(instance->protocol_name);

    // SubGhzTxRxWorker
    subghz_tx_rx_worker_free(instance->subghz_txrx);

    free(instance);
}

bool subbrute_worker_start(
    SubBruteWorker* instance,
    uint32_t frequency,
    FuriHalSubGhzPreset preset,
    const char* protocol_name) {
    furi_assert(instance);

    if(instance->worker_manual_mode) {
        FURI_LOG_W(TAG, "Invalid mode for starting worker!");
        return false;
    }

    instance->frequency = frequency;
    instance->preset = preset;

    string_clear(instance->protocol_name);
    string_init_printf(instance->protocol_name, "%s", protocol_name);

    bool res = false;

    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_preset(instance->preset);

    furi_hal_subghz_set_frequency_and_path(instance->frequency);
    furi_hal_subghz_flush_rx();

    if(furi_hal_subghz_is_tx_allowed(frequency)) {
        instance->frequency = frequency;
        res = true;
    }
    instance->worker_running = res;

#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Frequency: %d", frequency);
#endif
    instance->preset = preset;
    if(res) {
        instance->worker_running = res =
            subghz_tx_rx_worker_start(instance->subghz_txrx, frequency);
    }
    return res;
}

void subbrute_worker_stop(SubBruteWorker* instance) {
    furi_assert(instance);

    instance->worker_running = false;

    if(subghz_tx_rx_worker_is_running(instance->subghz_txrx)) {
        subghz_tx_rx_worker_stop(instance->subghz_txrx);
    }
}

void subbrute_worker_set_continuous_worker(SubBruteWorker* instance, bool is_continuous_worker) {
    furi_assert(instance);

    instance->is_continuous_worker = is_continuous_worker;
}

bool subbrute_worker_get_continuous_worker(SubBruteWorker* instance) {
    furi_assert(instance);

    return instance->is_continuous_worker;
}

bool subbrute_worker_is_running(SubBruteWorker* instance) {
    furi_assert(instance);

    return instance->worker_running;
}

bool subbrute_worker_can_transmit(SubBruteWorker* instance) {
    furi_assert(instance);

    return (furi_get_tick() - instance->last_time_tx_data) > SUBBRUTE_SEND_DELAY;
}

bool subbrute_worker_can_manual_transmit(SubBruteWorker* instance, bool is_button_pressed) {
    furi_assert(instance);

    if(is_button_pressed) {
        // It's human pressed, trying to reset twice pressing
        return !instance->worker_manual_mode &&
               (furi_get_tick() - instance->last_time_tx_data) > 500;
    } else {
        return !instance->worker_manual_mode;
    }
}

bool subbrute_worker_transmit(SubBruteWorker* instance, const char* payload) {
    furi_assert(instance);
    furi_assert(instance->worker_running);

    if(!subbrute_worker_can_transmit(instance)) {
        FURI_LOG_E(TAG, "Too early to transmit");

        return false;
    }
    instance->last_time_tx_data = furi_get_tick();

#ifdef FURI_DEBUG
    //FURI_LOG_D(TAG, "payload: %s", payload);
#endif

    while(!subghz_tx_rx_worker_write(instance->subghz_txrx, (uint8_t*)payload, strlen(payload))) {
        furi_delay_ms(10);
    }

    furi_hal_subghz_flush_tx();
    //    Stream* stream = flipper_format_get_raw_stream(instance->flipper_format);
    //    stream_clean(stream);
    //    stream_write_cstring(stream, payload);
    //    subghz_transmitter_deserialize(instance->transmitter, instance->flipper_format);

    return true;
}

// Init MANUAL
bool subbrute_worker_init_manual_transmit(
    SubBruteWorker* instance,
    uint32_t frequency,
    FuriHalSubGhzPreset preset,
    const char* protocol_name) {
#ifdef FURI_DEBUG
    FURI_LOG_D(
        TAG,
        "subbrute_worker_init_manual_transmit. frequency: %d, protocol: %s",
        frequency,
        protocol_name);
#endif
    if(instance->worker_manual_mode || !subbrute_worker_can_manual_transmit(instance, false) ||
       instance->worker_running) {
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "cannot transmit");
#endif
        return false;
    }
    if(instance->worker_running) {
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "subbrute_worker_stop");
#endif
        subbrute_worker_stop(instance);
    }

    // Not transmit at this period
    instance->worker_manual_mode = true;

    if(instance->is_manual_init) {
        FURI_LOG_E(TAG, "Trying to setup without normally shutdown prev transmit session!");
        subbrute_worker_manual_transmit_stop(instance);
    }

    instance->preset = preset;
    instance->frequency = frequency;

    string_clear(instance->protocol_name);
    string_init_printf(instance->protocol_name, "%s", protocol_name);

    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_preset(instance->preset);

    furi_hal_subghz_set_frequency_and_path(instance->frequency);
    furi_hal_subghz_flush_rx();

    if(!furi_hal_subghz_is_tx_allowed(frequency)) {
        FURI_LOG_E(TAG, "Frequency: %d invalid!", frequency);

        instance->frequency = frequency;
        instance->worker_manual_mode = false;
        return false;
    }

#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Frequency: %d", frequency);
#endif

    instance->transmitter = subghz_transmitter_alloc_init(
        instance->environment, string_get_cstr(instance->protocol_name));

    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(instance->preset);
    instance->frequency = furi_hal_subghz_set_frequency_and_path(frequency);

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();
    subghz_transmitter_free(instance->transmitter);
    instance->transmitter = NULL;

    instance->worker_manual_mode = false;
    instance->is_manual_init = true;

    return true;
}

void subbrute_worker_manual_transmit_stop(SubBruteWorker* instance) {
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_worker_manual_transmit_stop");
#endif
    if(!instance->is_manual_init) {
        return;
    }

    furi_hal_subghz_idle();
    furi_hal_subghz_sleep();

    if(instance->transmitter != NULL) {
        subghz_transmitter_free(instance->transmitter);
        instance->transmitter = NULL;
    }

    instance->is_manual_init = false;
}

bool subbrute_worker_manual_transmit(SubBruteWorker* instance, const char* payload) {
    furi_assert(instance);

    if(instance->worker_manual_mode || !subbrute_worker_can_transmit(instance)) {
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "cannot transmit");
#endif
        return false;
    }
    if(instance->worker_running) {
        FURI_LOG_W(TAG, "Worker was working for manual mode. Shutdown thread");
        subbrute_worker_stop(instance);
    }
    if(!instance->is_manual_init) {
        FURI_LOG_E(TAG, "Manually transmit doesn't set!");
        return false;
    }

    instance->last_time_tx_data = furi_get_tick();
    instance->worker_manual_mode = true;

    Stream* stream = flipper_format_get_raw_stream(instance->flipper_format);
    stream_clean(stream);
    stream_write_cstring(stream, payload);

    instance->transmitter = subghz_transmitter_alloc_init(
        instance->environment, string_get_cstr(instance->protocol_name));
    subghz_transmitter_deserialize(instance->transmitter, instance->flipper_format);
    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(instance->preset);
    instance->frequency = furi_hal_subghz_set_frequency_and_path(instance->frequency);

    furi_hal_subghz_start_async_tx(subghz_transmitter_yield, instance->transmitter);

    while(!furi_hal_subghz_is_async_tx_complete()) {
        furi_delay_ms(SUBBRUTE_TX_TIMEOUT);
    }
    furi_hal_subghz_stop_async_tx();

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();
    subghz_transmitter_free(instance->transmitter);
    instance->transmitter = NULL;

    stream_clean(stream);

    instance->worker_manual_mode = false;

    return true;
}