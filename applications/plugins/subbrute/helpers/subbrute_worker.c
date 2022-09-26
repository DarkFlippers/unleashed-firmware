#include "subbrute_worker.h"

#include <subghz/environment.h>
#include <subghz/transmitter.h>
#include <flipper_format_i.h>

#define TAG "SubBruteWorker"

struct SubBruteWorker {
    FuriThread* thread;
    volatile bool worker_running;
    volatile bool worker_manual_mode;
    bool is_manual_init;

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
#define SUBBRUTE_TX_TIMEOUT 1
#define SUBBRUTE_SEND_DELAY 260

/**
 * Entrypoint for worker
 *
 * @param context SubBruteWorker*
 * @return 0 if ok
 */
int32_t subbrute_worker_thread(void* context) {
    furi_assert(context);
    SubBruteWorker* instance = (SubBruteWorker*)context;

    if(!instance->worker_running) {
        FURI_LOG_W(TAG, "Worker is not set to running state!");
        return -1;
    }
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Worker start");
#endif

    //instance->environment = subghz_environment_alloc();
    instance->transmitter = subghz_transmitter_alloc_init(
        instance->environment, string_get_cstr(instance->protocol_name));

    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(instance->preset);
    instance->frequency = furi_hal_subghz_set_frequency_and_path(instance->frequency);

    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_cc1101_g0, true);

    // Set ready to transmit value
    //instance->last_time_tx_data = furi_get_tick() - SUBBRUTE_SEND_DELAY;

    while(instance->worker_running) {
        // Transmit
        if(!furi_hal_subghz_tx()) {
            FURI_LOG_E(TAG, "Cannot transmit!");
            break;
        }
        furi_delay_ms(SUBBRUTE_TX_TIMEOUT);
    }

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();

    subghz_transmitter_free(instance->transmitter);
    instance->transmitter = NULL;
    /*subghz_environment_free(instance->environment);
    instance->environment = NULL;*/

#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Worker stop");
#endif
    return 0;
}

SubBruteWorker* subbrute_worker_alloc() {
    SubBruteWorker* instance = malloc(sizeof(SubBruteWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "SubBruteAttackWorker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subbrute_worker_thread);

    //instance->status = SubBruteWorkerStatusIDLE;
    instance->worker_running = false;
    instance->worker_manual_mode = false;

    instance->flipper_format = flipper_format_string_alloc();
    string_init(instance->protocol_name);

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

    furi_thread_free(instance->thread);
    flipper_format_free(instance->flipper_format);

    string_clear(instance->protocol_name);

    free(instance);
}

bool subbrute_worker_start(
    SubBruteWorker* instance,
    uint32_t frequency,
    FuriHalSubGhzPreset preset,
    const char* protocol_name) {
    furi_assert(instance);

    if(instance->worker_manual_mode) {
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

    furi_thread_start(instance->thread);

    return res;
}

void subbrute_worker_stop(SubBruteWorker* instance) {
    furi_assert(instance);

    instance->worker_running = false;

    furi_thread_join(instance->thread);

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();
}

bool subbrute_worker_is_running(SubBruteWorker* instance) {
    furi_assert(instance);

    return instance->worker_running;
}

bool subbrute_worker_can_transmit(SubBruteWorker* instance) {
    UNUSED(instance);
    return true;
    //furi_assert(instance);
    //return (furi_get_tick() - instance->last_time_tx_data) > SUBBRUTE_SEND_DELAY;
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

    Stream* stream = flipper_format_get_raw_stream(instance->flipper_format);
    stream_clean(stream);
    stream_write_cstring(stream, payload);
    subghz_transmitter_deserialize(instance->transmitter, instance->flipper_format);

    return true;
}

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
    if(instance->worker_manual_mode || !subbrute_worker_can_transmit(instance)) {
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

    //instance->environment = subghz_environment_alloc();
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
    /*subghz_environment_free(instance->environment);
    instance->environment = NULL;*/

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
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "subbrute_worker_stop");
#endif
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