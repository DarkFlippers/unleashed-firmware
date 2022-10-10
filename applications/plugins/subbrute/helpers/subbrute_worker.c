#include "subbrute_worker_private.h"
#include <string.h>
#include <toolbox/stream/stream.h>
#include <flipper_format.h>
#include <flipper_format_i.h>

#define TAG "SubBruteWorker"
#define SUBBRUTE_TX_TIMEOUT 5
#define SUBBRUTE_MANUAL_TRANSMIT_INTERVAL 400

SubBruteWorker* subbrute_worker_alloc() {
    SubBruteWorker* instance = malloc(sizeof(SubBruteWorker));

    instance->state = SubBruteWorkerStateIDLE;
    instance->step = 0;
    instance->worker_running = false;
    instance->initiated = false;
    instance->last_time_tx_data = 0;
    instance->load_index = 0;

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "SubBruteAttackWorker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subbrute_worker_thread);

    instance->context = NULL;
    instance->callback = NULL;

    instance->decoder_result = NULL;
    instance->transmitter = NULL;
    instance->environment = subghz_environment_alloc();

    instance->transmit_mode = false;

    return instance;
}

void subbrute_worker_free(SubBruteWorker* instance) {
    furi_assert(instance);

    // I don't know how to free this
    instance->decoder_result = NULL;

    if(instance->transmitter != NULL) {
        subghz_transmitter_free(instance->transmitter);
        instance->transmitter = NULL;
    }

    subghz_environment_free(instance->environment);
    instance->environment = NULL;

    furi_thread_free(instance->thread);

    free(instance);
}

uint64_t subbrute_worker_get_step(SubBruteWorker* instance) {
    return instance->step;
}

bool subbrute_worker_set_step(SubBruteWorker* instance, uint64_t step) {
    furi_assert(instance);
    if(!subbrute_worker_can_manual_transmit(instance)) {
        FURI_LOG_W(TAG, "Cannot set step during running mode");
        return false;
    }

    instance->step = step;

    return true;
}

bool subbrute_worker_init_default_attack(
    SubBruteWorker* instance,
    SubBruteAttacks attack_type,
    uint64_t step,
    const SubBruteProtocol* protocol) {
    furi_assert(instance);

    if(instance->worker_running) {
        FURI_LOG_W(TAG, "Init Worker when it's running");
        subbrute_worker_stop(instance);
    }

    instance->attack = attack_type;
    instance->frequency = protocol->frequency;
    instance->preset = protocol->preset;
    instance->file = protocol->file;
    instance->step = step;
    instance->bits = protocol->bits;
    instance->te = protocol->te;
    instance->repeat = protocol->repeat;
    instance->load_index = 0;
    instance->file_key = NULL;
    instance->max_value = subbrute_protocol_calc_max_value(instance->attack, instance->bits);

    instance->initiated = true;
    instance->state = SubBruteWorkerStateReady;
    subbrute_worker_send_callback(instance);
#ifdef FURI_DEBUG
    FURI_LOG_I(
        TAG,
        "subbrute_worker_init_default_attack: %s, bits: %d, preset: %s, file: %s, te: %d, repeat: %d, max_value: %lld",
        subbrute_protocol_name(instance->attack),
        instance->bits,
        subbrute_protocol_preset(instance->preset),
        subbrute_protocol_file(instance->file),
        instance->te,
        instance->repeat,
        instance->max_value);
#endif

    return true;
}

bool subbrute_worker_init_file_attack(
    SubBruteWorker* instance,
    uint64_t step,
    uint8_t load_index,
    const char* file_key,
    SubBruteProtocol* protocol) {
    furi_assert(instance);

    if(instance->worker_running) {
        FURI_LOG_W(TAG, "Init Worker when it's running");
        subbrute_worker_stop(instance);
    }

    instance->attack = SubBruteAttackLoadFile;
    instance->frequency = protocol->frequency;
    instance->preset = protocol->preset;
    instance->file = protocol->file;
    instance->step = step;
    instance->bits = protocol->bits;
    instance->te = protocol->te;
    instance->load_index = load_index;
    instance->repeat = protocol->repeat;
    instance->file_key = file_key;
    instance->max_value = subbrute_protocol_calc_max_value(instance->attack, instance->bits);

    instance->initiated = true;
    instance->state = SubBruteWorkerStateReady;
    subbrute_worker_send_callback(instance);
#ifdef FURI_DEBUG
    FURI_LOG_I(
        TAG,
        "subbrute_worker_init_file_attack: %s, bits: %d, preset: %s, file: %s, te: %d, repeat: %d, max_value: %lld",
        subbrute_protocol_name(instance->attack),
        instance->bits,
        subbrute_protocol_preset(instance->preset),
        subbrute_protocol_file(instance->file),
        instance->te,
        instance->repeat,
        instance->max_value);
#endif

    return true;
}

bool subbrute_worker_start(SubBruteWorker* instance) {
    furi_assert(instance);

    if(!instance->initiated) {
        FURI_LOG_W(TAG, "Worker not init!");
        return false;
    }

    if(instance->worker_running) {
        FURI_LOG_W(TAG, "Worker is already running!");
        return false;
    }
    if(instance->state != SubBruteWorkerStateReady &&
       instance->state != SubBruteWorkerStateFinished) {
        FURI_LOG_W(TAG, "Worker cannot start, invalid device state: %d", instance->state);
        return false;
    }

    instance->worker_running = true;
    furi_thread_start(instance->thread);

    return true;
}

void subbrute_worker_stop(SubBruteWorker* instance) {
    furi_assert(instance);

    instance->worker_running = false;

    furi_thread_join(instance->thread);

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();
}

bool subbrute_worker_transmit_current_key(SubBruteWorker* instance, uint64_t step) {
    furi_assert(instance);

    if(!instance->initiated) {
        FURI_LOG_W(TAG, "Worker not init!");
        return false;
    }
    if(instance->worker_running) {
        FURI_LOG_W(TAG, "Worker in running state!");
        return false;
    }
    if(instance->state != SubBruteWorkerStateReady &&
       instance->state != SubBruteWorkerStateFinished) {
        FURI_LOG_W(TAG, "Invalid state for running worker! State: %d", instance->state);
        return false;
    }

    uint32_t ticks = furi_get_tick();
    if((ticks - instance->last_time_tx_data) < SUBBRUTE_MANUAL_TRANSMIT_INTERVAL) {
#if FURI_DEBUG
        FURI_LOG_D(TAG, "Need to wait, current: %ld", ticks - instance->last_time_tx_data);
#endif
        return false;
    }

    instance->last_time_tx_data = ticks;
    instance->step = step;

    bool result;
    instance->protocol_name = subbrute_protocol_file(instance->file);
    FlipperFormat* flipper_format = flipper_format_string_alloc();
    Stream* stream = flipper_format_get_raw_stream(flipper_format);

    stream_clean(stream);

    if(instance->attack == SubBruteAttackLoadFile) {
        subbrute_protocol_file_payload(
            stream,
            step,
            instance->bits,
            instance->te,
            instance->repeat,
            instance->load_index,
            instance->file_key);
    } else {
        subbrute_protocol_default_payload(
            stream, step, instance->bits, instance->te, instance->repeat);
    }

    //    size_t written = stream_write_string(stream, payload);
    //    if(written <= 0) {
    //        FURI_LOG_W(TAG, "Error creating packet! EXIT");
    //        result = false;
    //    } else {
    subbrute_worker_subghz_transmit(instance, flipper_format);

    result = true;
#if FURI_DEBUG
    FURI_LOG_D(TAG, "Manual transmit done");
#endif
    //    }

    flipper_format_free(flipper_format);
    //    furi_string_free(payload);

    return result;
}

bool subbrute_worker_is_running(SubBruteWorker* instance) {
    return instance->worker_running;
}

bool subbrute_worker_can_manual_transmit(SubBruteWorker* instance) {
    furi_assert(instance);

    if(!instance->initiated) {
        FURI_LOG_W(TAG, "Worker not init!");
        return false;
    }

    return !instance->worker_running && instance->state != SubBruteWorkerStateIDLE &&
           instance->state != SubBruteWorkerStateTx &&
           ((furi_get_tick() - instance->last_time_tx_data) > SUBBRUTE_MANUAL_TRANSMIT_INTERVAL);
}

void subbrute_worker_set_callback(
    SubBruteWorker* instance,
    SubBruteWorkerCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

void subbrute_worker_subghz_transmit(SubBruteWorker* instance, FlipperFormat* flipper_format) {
    while(instance->transmit_mode) {
        furi_delay_ms(SUBBRUTE_TX_TIMEOUT);
    }
    instance->transmit_mode = true;
    if(instance->transmitter != NULL) {
        subghz_transmitter_free(instance->transmitter);
        instance->transmitter = NULL;
    }
    instance->transmitter =
        subghz_transmitter_alloc_init(instance->environment, instance->protocol_name);
    subghz_transmitter_deserialize(instance->transmitter, flipper_format);
    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(instance->preset);
    furi_hal_subghz_set_frequency_and_path(instance->frequency);
    furi_hal_subghz_start_async_tx(subghz_transmitter_yield, instance->transmitter);

    while(!furi_hal_subghz_is_async_tx_complete()) {
        furi_delay_ms(SUBBRUTE_TX_TIMEOUT);
    }
    furi_hal_subghz_stop_async_tx();

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();
    subghz_transmitter_free(instance->transmitter);
    instance->transmitter = NULL;

    instance->transmit_mode = false;
}

void subbrute_worker_send_callback(SubBruteWorker* instance) {
    if(instance->callback != NULL) {
        instance->callback(instance->context, instance->state);
    }
}

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
    if(instance->state != SubBruteWorkerStateReady &&
       instance->state != SubBruteWorkerStateFinished) {
        FURI_LOG_W(TAG, "Invalid state for running worker! State: %d", instance->state);
        return -2;
    }
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Worker start");
#endif

    SubBruteWorkerState local_state = instance->state = SubBruteWorkerStateTx;
    subbrute_worker_send_callback(instance);

    instance->protocol_name = subbrute_protocol_file(instance->file);

    FlipperFormat* flipper_format = flipper_format_string_alloc();
    Stream* stream = flipper_format_get_raw_stream(flipper_format);

    while(instance->worker_running) {
        stream_clean(stream);
        if(instance->attack == SubBruteAttackLoadFile) {
            subbrute_protocol_file_payload(
                stream,
                instance->step,
                instance->bits,
                instance->te,
                instance->repeat,
                instance->load_index,
                instance->file_key);
        } else {
            subbrute_protocol_default_payload(
                stream, instance->step, instance->bits, instance->te, instance->repeat);
        }
#ifdef FURI_DEBUG
        //FURI_LOG_I(TAG, "Payload: %s", furi_string_get_cstr(payload));
        //furi_delay_ms(SUBBRUTE_MANUAL_TRANSMIT_INTERVAL / 4);
#endif

        //        size_t written = stream_write_stream_write_string(stream, payload);
        //        if(written <= 0) {
        //            FURI_LOG_W(TAG, "Error creating packet! BREAK");
        //            instance->worker_running = false;
        //            local_state = SubBruteWorkerStateIDLE;
        //            furi_string_free(payload);
        //            break;
        //        }

        subbrute_worker_subghz_transmit(instance, flipper_format);

        if(instance->step + 1 > instance->max_value) {
#ifdef FURI_DEBUG
            FURI_LOG_I(TAG, "Worker finished to end");
#endif
            local_state = SubBruteWorkerStateFinished;
            //            furi_string_free(payload);
            break;
        }
        instance->step++;

        //        furi_string_free(payload);
        furi_delay_ms(SUBBRUTE_TX_TIMEOUT);
    }

    flipper_format_free(flipper_format);

    instance->worker_running = false; // Because we have error states
    instance->state = local_state == SubBruteWorkerStateTx ? SubBruteWorkerStateReady :
                                                             local_state;
    subbrute_worker_send_callback(instance);

#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Worker stop");
#endif
    return 0;
}
