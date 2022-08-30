#include <furi.h>
#include <furi_hal.h>
#include "lfrfid_worker_i.h"
#include "tools/t5577.h"
#include <stream_buffer.h>
#include <toolbox/pulse_protocols/pulse_glue.h>
#include <toolbox/buffer_stream.h>
#include "tools/varint_pair.h"
#include "tools/bit_lib.h"

#define TAG "LFRFIDWorker"

/**
 * if READ_DEBUG_GPIO is defined:
 *     gpio_ext_pa7 will repeat signal coming from the comparator
 *     gpio_ext_pa6 will show load on the decoder
 */
// #define LFRFID_WORKER_READ_DEBUG_GPIO 1

#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
#define LFRFID_WORKER_READ_DEBUG_GPIO_VALUE &gpio_ext_pa7
#define LFRFID_WORKER_READ_DEBUG_GPIO_LOAD &gpio_ext_pa6
#endif

#define LFRFID_WORKER_READ_AVERAGE_COUNT 64
#define LFRFID_WORKER_READ_MIN_TIME_US 16

#define LFRFID_WORKER_READ_DROP_TIME_MS 50
#define LFRFID_WORKER_READ_STABILIZE_TIME_MS 450
#define LFRFID_WORKER_READ_SWITCH_TIME_MS 2000

#define LFRFID_WORKER_WRITE_VERIFY_TIME_MS 2000
#define LFRFID_WORKER_WRITE_DROP_TIME_MS 50
#define LFRFID_WORKER_WRITE_TOO_LONG_TIME_MS 10000

#define LFRFID_WORKER_WRITE_MAX_UNSUCCESSFUL_READS 5

#define LFRFID_WORKER_READ_BUFFER_SIZE 512
#define LFRFID_WORKER_READ_BUFFER_COUNT 16

#define LFRFID_WORKER_EMULATE_BUFFER_SIZE 1024

#define LFRFID_WORKER_DELAY_QUANT 50

void lfrfid_worker_delay(LFRFIDWorker* worker, uint32_t milliseconds) {
    for(uint32_t i = 0; i < (milliseconds / LFRFID_WORKER_DELAY_QUANT); i++) {
        if(lfrfid_worker_check_for_stop(worker)) break;
        furi_delay_ms(LFRFID_WORKER_DELAY_QUANT);
    }
}

/**************************************************************************************************/
/********************************************** READ **********************************************/
/**************************************************************************************************/

typedef struct {
    BufferStream* stream;
    VarintPair* pair;
    bool ignore_next_pulse;
} LFRFIDWorkerReadContext;

static void lfrfid_worker_read_capture(bool level, uint32_t duration, void* context) {
    LFRFIDWorkerReadContext* ctx = context;

    // ignore pulse if last pulse was noise
    if(ctx->ignore_next_pulse) {
        ctx->ignore_next_pulse = false;
        return;
    }

    // ignore noise spikes
    if(duration <= LFRFID_WORKER_READ_MIN_TIME_US) {
        if(level) {
            ctx->ignore_next_pulse = true;
        }
        varint_pair_reset(ctx->pair);
        return;
    }

#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
    furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_VALUE, level);
#endif

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool need_to_send = varint_pair_pack(ctx->pair, level, duration);
    if(need_to_send) {
        buffer_stream_send_from_isr(
            ctx->stream,
            varint_pair_get_data(ctx->pair),
            varint_pair_get_size(ctx->pair),
            &xHigherPriorityTaskWoken);
        varint_pair_reset(ctx->pair);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

typedef enum {
    LFRFIDWorkerReadOK,
    LFRFIDWorkerReadExit,
    LFRFIDWorkerReadTimeout,
} LFRFIDWorkerReadState;

static LFRFIDWorkerReadState lfrfid_worker_read_internal(
    LFRFIDWorker* worker,
    LFRFIDFeature feature,
    uint32_t timeout,
    ProtocolId* result_protocol) {
    LFRFIDWorkerReadState state = LFRFIDWorkerReadTimeout;
    furi_hal_rfid_pins_read();

    if(feature & LFRFIDFeatureASK) {
        furi_hal_rfid_tim_read(125000, 0.5);
        FURI_LOG_D(TAG, "Start ASK");
        if(worker->read_cb) {
            worker->read_cb(LFRFIDWorkerReadStartASK, PROTOCOL_NO, worker->cb_ctx);
        }
    } else {
        furi_hal_rfid_tim_read(62500, 0.25);
        FURI_LOG_D(TAG, "Start PSK");
        if(worker->read_cb) {
            worker->read_cb(LFRFIDWorkerReadStartPSK, PROTOCOL_NO, worker->cb_ctx);
        }
    }

    furi_hal_rfid_tim_read_start();

    // stabilize detector
    lfrfid_worker_delay(worker, LFRFID_WORKER_READ_STABILIZE_TIME_MS);

    protocol_dict_decoders_start(worker->protocols);

#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
    furi_hal_gpio_init_simple(LFRFID_WORKER_READ_DEBUG_GPIO_VALUE, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, GpioModeOutputPushPull);
    furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_VALUE, false);
    furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, false);
#endif

    LFRFIDWorkerReadContext ctx;
    ctx.pair = varint_pair_alloc();
    ctx.stream =
        buffer_stream_alloc(LFRFID_WORKER_READ_BUFFER_SIZE, LFRFID_WORKER_READ_BUFFER_COUNT);

    furi_hal_rfid_tim_read_capture_start(lfrfid_worker_read_capture, &ctx);

    *result_protocol = PROTOCOL_NO;
    ProtocolId last_protocol = PROTOCOL_NO;
    size_t last_size = protocol_dict_get_max_data_size(worker->protocols);
    uint8_t* last_data = malloc(last_size);
    uint8_t* protocol_data = malloc(last_size);
    size_t last_read_count = 0;

    uint32_t switch_os_tick_last = furi_get_tick();

    uint32_t average_duration = 0;
    uint32_t average_pulse = 0;
    size_t average_index = 0;
    bool card_detected = false;

    FURI_LOG_D(TAG, "Read started");
    while(true) {
        if(lfrfid_worker_check_for_stop(worker)) {
            state = LFRFIDWorkerReadExit;
            break;
        }

        Buffer* buffer = buffer_stream_receive(ctx.stream, 100);

#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
        furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, true);
#endif

        if(buffer_stream_get_overrun_count(ctx.stream) > 0) {
            FURI_LOG_E(TAG, "Read overrun, recovering");
            buffer_stream_reset(ctx.stream);
#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
            furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, false);
#endif
            continue;
        }

        if(buffer == NULL) {
#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
            furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, false);
#endif
            continue;
        }

        size_t size = buffer_get_size(buffer);
        uint8_t* data = buffer_get_data(buffer);
        size_t index = 0;

        while(index < size) {
            uint32_t duration;
            uint32_t pulse;
            size_t tmp_size;

            if(!varint_pair_unpack(&data[index], size - index, &pulse, &duration, &tmp_size)) {
                FURI_LOG_E(TAG, "can't unpack varint pair");
                break;
            } else {
                index += tmp_size;

                average_duration += duration;
                average_pulse += pulse;
                average_index++;
                if(average_index >= LFRFID_WORKER_READ_AVERAGE_COUNT) {
                    float average = (float)average_pulse / (float)average_duration;
                    average_pulse = 0;
                    average_duration = 0;
                    average_index = 0;

                    if(worker->read_cb) {
                        if(average > 0.2 && average < 0.8) {
                            if(!card_detected) {
                                card_detected = true;
                                worker->read_cb(
                                    LFRFIDWorkerReadSenseStart, PROTOCOL_NO, worker->cb_ctx);
                            }
                        } else {
                            if(card_detected) {
                                card_detected = false;
                                worker->read_cb(
                                    LFRFIDWorkerReadSenseEnd, PROTOCOL_NO, worker->cb_ctx);
                            }
                        }
                    }
                }

                ProtocolId protocol = PROTOCOL_NO;

                protocol = protocol_dict_decoders_feed_by_feature(
                    worker->protocols, feature, true, pulse);
                if(protocol == PROTOCOL_NO) {
                    protocol = protocol_dict_decoders_feed_by_feature(
                        worker->protocols, feature, false, duration - pulse);
                }

                if(protocol != PROTOCOL_NO) {
                    // reset switch timer
                    switch_os_tick_last = furi_get_tick();

                    size_t protocol_data_size =
                        protocol_dict_get_data_size(worker->protocols, protocol);
                    protocol_dict_get_data(
                        worker->protocols, protocol, protocol_data, protocol_data_size);

                    // validate protocol
                    if(protocol == last_protocol &&
                       memcmp(last_data, protocol_data, protocol_data_size) == 0) {
                        last_read_count = last_read_count + 1;

                        size_t validation_count =
                            protocol_dict_get_validate_count(worker->protocols, protocol);

                        if(last_read_count >= validation_count) {
                            state = LFRFIDWorkerReadOK;
                            *result_protocol = protocol;
                            break;
                        }
                    } else {
                        if(last_protocol == PROTOCOL_NO && worker->read_cb) {
                            worker->read_cb(
                                LFRFIDWorkerReadSenseCardStart, protocol, worker->cb_ctx);
                        }

                        last_protocol = protocol;
                        memcpy(last_data, protocol_data, protocol_data_size);
                        last_read_count = 0;
                    }

                    if(furi_log_get_level() >= FuriLogLevelDebug) {
                        string_t string_info;
                        string_init(string_info);
                        for(uint8_t i = 0; i < protocol_data_size; i++) {
                            if(i != 0) {
                                string_cat_printf(string_info, " ");
                            }

                            string_cat_printf(string_info, "%02X", protocol_data[i]);
                        }

                        FURI_LOG_D(
                            TAG,
                            "%s, %d, [%s]",
                            protocol_dict_get_name(worker->protocols, protocol),
                            last_read_count,
                            string_get_cstr(string_info));
                        string_clear(string_info);
                    }

                    protocol_dict_decoders_start(worker->protocols);
                }
            }
        }

        buffer_reset(buffer);

#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
        furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, false);
#endif

        if(*result_protocol != PROTOCOL_NO) {
            break;
        }

        if((furi_get_tick() - switch_os_tick_last) > timeout) {
            state = LFRFIDWorkerReadTimeout;
            break;
        }
    }

    FURI_LOG_D(TAG, "Read stopped");

    if(last_protocol != PROTOCOL_NO && worker->read_cb) {
        worker->read_cb(LFRFIDWorkerReadSenseCardEnd, last_protocol, worker->cb_ctx);
    }

    if(card_detected && worker->read_cb) {
        worker->read_cb(LFRFIDWorkerReadSenseEnd, last_protocol, worker->cb_ctx);
    }

    furi_hal_rfid_tim_read_capture_stop();
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();

    varint_pair_free(ctx.pair);
    buffer_stream_free(ctx.stream);

    free(protocol_data);
    free(last_data);

#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
    furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_VALUE, false);
    furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, false);
    furi_hal_gpio_init_simple(LFRFID_WORKER_READ_DEBUG_GPIO_VALUE, GpioModeAnalog);
    furi_hal_gpio_init_simple(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, GpioModeAnalog);
#endif

    return state;
}

static void lfrfid_worker_mode_read_process(LFRFIDWorker* worker) {
    LFRFIDFeature feature = LFRFIDFeatureASK;
    ProtocolId read_result = PROTOCOL_NO;
    LFRFIDWorkerReadState state;

    if(worker->read_type == LFRFIDWorkerReadTypePSKOnly) {
        feature = LFRFIDFeaturePSK;
    } else {
        feature = LFRFIDFeatureASK;
    }

    if(worker->read_type == LFRFIDWorkerReadTypeAuto) {
        while(1) {
            // read for a while
            state = lfrfid_worker_read_internal(
                worker, feature, LFRFID_WORKER_READ_SWITCH_TIME_MS, &read_result);

            if(state == LFRFIDWorkerReadOK || state == LFRFIDWorkerReadExit) {
                break;
            }

            // switch to next feature
            if(feature == LFRFIDFeatureASK) {
                feature = LFRFIDFeaturePSK;
            } else {
                feature = LFRFIDFeatureASK;
            }

            lfrfid_worker_delay(worker, LFRFID_WORKER_READ_DROP_TIME_MS);
        }
    } else {
        while(1) {
            if(worker->read_type == LFRFIDWorkerReadTypeASKOnly) {
                state = lfrfid_worker_read_internal(worker, feature, UINT32_MAX, &read_result);
            } else {
                state = lfrfid_worker_read_internal(
                    worker, feature, LFRFID_WORKER_READ_SWITCH_TIME_MS, &read_result);
            }

            if(state == LFRFIDWorkerReadOK || state == LFRFIDWorkerReadExit) {
                break;
            }

            lfrfid_worker_delay(worker, LFRFID_WORKER_READ_DROP_TIME_MS);
        }
    }

    if(state == LFRFIDWorkerReadOK && worker->read_cb) {
        worker->read_cb(LFRFIDWorkerReadDone, read_result, worker->cb_ctx);
    }
}

/**************************************************************************************************/
/******************************************** EMULATE *********************************************/
/**************************************************************************************************/

typedef struct {
    uint32_t duration[LFRFID_WORKER_EMULATE_BUFFER_SIZE];
    uint32_t pulse[LFRFID_WORKER_EMULATE_BUFFER_SIZE];
} LFRFIDWorkerEmulateBuffer;

typedef enum {
    HalfTransfer,
    TransferComplete,
} LFRFIDWorkerEmulateDMAEvent;

static void lfrfid_worker_emulate_dma_isr(bool half, void* context) {
    StreamBufferHandle_t stream = context;
    uint32_t flag = half ? HalfTransfer : TransferComplete;
    xStreamBufferSendFromISR(stream, &flag, sizeof(uint32_t), pdFALSE);
}

static void lfrfid_worker_mode_emulate_process(LFRFIDWorker* worker) {
    LFRFIDWorkerEmulateBuffer* buffer = malloc(sizeof(LFRFIDWorkerEmulateBuffer));
    StreamBufferHandle_t stream = xStreamBufferCreate(sizeof(uint32_t), sizeof(uint32_t));
    LFRFIDProtocol protocol = worker->protocol;
    PulseGlue* pulse_glue = pulse_glue_alloc();

    protocol_dict_encoder_start(worker->protocols, protocol);

    for(size_t i = 0; i < LFRFID_WORKER_EMULATE_BUFFER_SIZE; i++) {
        bool pulse_pop = false;
        while(!pulse_pop) {
            LevelDuration level_duration =
                protocol_dict_encoder_yield(worker->protocols, protocol);
            pulse_pop = pulse_glue_push(
                pulse_glue,
                level_duration_get_level(level_duration),
                level_duration_get_duration(level_duration));
        }
        uint32_t duration, pulse;
        pulse_glue_pop(pulse_glue, &duration, &pulse);
        buffer->duration[i] = duration - 1;
        buffer->pulse[i] = pulse;
    }

#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
    furi_hal_gpio_init_simple(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, GpioModeOutputPushPull);
#endif

    furi_hal_rfid_tim_emulate_dma_start(
        buffer->duration,
        buffer->pulse,
        LFRFID_WORKER_EMULATE_BUFFER_SIZE,
        lfrfid_worker_emulate_dma_isr,
        stream);

    while(true) {
        uint32_t flag = 0;
        size_t size = xStreamBufferReceive(stream, &flag, sizeof(uint32_t), 100);

#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
        furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, true);
#endif

        if(size == sizeof(uint32_t)) {
            size_t start = 0;

            if(flag == HalfTransfer) {
                start = 0;
            } else if(flag == TransferComplete) {
                start = (LFRFID_WORKER_EMULATE_BUFFER_SIZE / 2);
            }

            for(size_t i = 0; i < (LFRFID_WORKER_EMULATE_BUFFER_SIZE / 2); i++) {
                bool pulse_pop = false;
                while(!pulse_pop) {
                    LevelDuration level_duration =
                        protocol_dict_encoder_yield(worker->protocols, protocol);
                    pulse_pop = pulse_glue_push(
                        pulse_glue,
                        level_duration_get_level(level_duration),
                        level_duration_get_duration(level_duration));
                }
                uint32_t duration, pulse;
                pulse_glue_pop(pulse_glue, &duration, &pulse);
                buffer->duration[start + i] = duration - 1;
                buffer->pulse[start + i] = pulse;
            }
        }

        if(lfrfid_worker_check_for_stop(worker)) {
            break;
        }

#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
        furi_hal_gpio_write(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, false);
#endif
    }

    furi_hal_rfid_tim_emulate_dma_stop();

#ifdef LFRFID_WORKER_READ_DEBUG_GPIO
    furi_hal_gpio_init_simple(LFRFID_WORKER_READ_DEBUG_GPIO_LOAD, GpioModeAnalog);
#endif

    free(buffer);
    vStreamBufferDelete(stream);
    pulse_glue_free(pulse_glue);
}

/**************************************************************************************************/
/********************************************* WRITE **********************************************/
/**************************************************************************************************/

static void lfrfid_worker_mode_write_process(LFRFIDWorker* worker) {
    LFRFIDProtocol protocol = worker->protocol;
    LFRFIDWriteRequest* request = malloc(sizeof(LFRFIDWriteRequest));
    request->write_type = LFRFIDWriteTypeT5577;

    bool can_be_written = protocol_dict_get_write_data(worker->protocols, protocol, request);

    uint32_t write_start_time = furi_get_tick();
    bool too_long = false;
    size_t unsuccessful_reads = 0;

    size_t data_size = protocol_dict_get_data_size(worker->protocols, protocol);
    uint8_t* verify_data = malloc(data_size);
    uint8_t* read_data = malloc(data_size);
    protocol_dict_get_data(worker->protocols, protocol, verify_data, data_size);

    if(can_be_written) {
        while(!lfrfid_worker_check_for_stop(worker)) {
            FURI_LOG_D(TAG, "Data write");
            t5577_write(&request->t5577);

            ProtocolId read_result = PROTOCOL_NO;
            LFRFIDWorkerReadState state = lfrfid_worker_read_internal(
                worker,
                protocol_dict_get_features(worker->protocols, protocol),
                LFRFID_WORKER_WRITE_VERIFY_TIME_MS,
                &read_result);

            if(state == LFRFIDWorkerReadOK) {
                bool read_success = false;

                if(read_result == protocol) {
                    protocol_dict_get_data(worker->protocols, protocol, read_data, data_size);

                    if(memcmp(read_data, verify_data, data_size) == 0) {
                        read_success = true;
                    }
                }

                if(read_success) {
                    if(worker->write_cb) {
                        worker->write_cb(LFRFIDWorkerWriteOK, worker->cb_ctx);
                    }
                    break;
                } else {
                    unsuccessful_reads++;

                    if(unsuccessful_reads == LFRFID_WORKER_WRITE_MAX_UNSUCCESSFUL_READS) {
                        if(worker->write_cb) {
                            worker->write_cb(LFRFIDWorkerWriteFobCannotBeWritten, worker->cb_ctx);
                        }
                    }
                }
            } else if(state == LFRFIDWorkerReadExit) {
                break;
            }

            if(!too_long &&
               (furi_get_tick() - write_start_time) > LFRFID_WORKER_WRITE_TOO_LONG_TIME_MS) {
                too_long = true;
                if(worker->write_cb) {
                    worker->write_cb(LFRFIDWorkerWriteTooLongToWrite, worker->cb_ctx);
                }
            }

            lfrfid_worker_delay(worker, LFRFID_WORKER_WRITE_DROP_TIME_MS);
        }
    } else {
        if(worker->write_cb) {
            worker->write_cb(LFRFIDWorkerWriteProtocolCannotBeWritten, worker->cb_ctx);
        }
    }

    free(request);
    free(verify_data);
    free(read_data);
}

/**************************************************************************************************/
/******************************************* READ RAW *********************************************/
/**************************************************************************************************/

static void lfrfid_worker_mode_read_raw_process(LFRFIDWorker* worker) {
    LFRFIDRawWorker* raw_worker = lfrfid_raw_worker_alloc();

    switch(worker->read_type) {
    case LFRFIDWorkerReadTypePSKOnly:
        lfrfid_raw_worker_start_read(
            raw_worker, worker->raw_filename, 62500, 0.25, worker->read_raw_cb, worker->cb_ctx);
        break;
    case LFRFIDWorkerReadTypeASKOnly:
        lfrfid_raw_worker_start_read(
            raw_worker, worker->raw_filename, 125000, 0.5, worker->read_raw_cb, worker->cb_ctx);
        break;
    default:
        furi_crash("RAW can be only PSK or ASK");
        break;
    }

    while(!lfrfid_worker_check_for_stop(worker)) {
        furi_delay_ms(100);
    }

    lfrfid_raw_worker_stop(raw_worker);
    lfrfid_raw_worker_free(raw_worker);
}

/**************************************************************************************************/
/***************************************** EMULATE RAW ********************************************/
/**************************************************************************************************/

static void lfrfid_worker_mode_emulate_raw_process(LFRFIDWorker* worker) {
    LFRFIDRawWorker* raw_worker = lfrfid_raw_worker_alloc();

    lfrfid_raw_worker_start_emulate(
        raw_worker, worker->raw_filename, worker->emulate_raw_cb, worker->cb_ctx);

    while(!lfrfid_worker_check_for_stop(worker)) {
        furi_delay_ms(100);
    }

    lfrfid_raw_worker_stop(raw_worker);
    lfrfid_raw_worker_free(raw_worker);
}

/**************************************************************************************************/
/******************************************** MODES ***********************************************/
/**************************************************************************************************/

const LFRFIDWorkerModeType lfrfid_worker_modes[] = {
    [LFRFIDWorkerIdle] = {.process = NULL},
    [LFRFIDWorkerRead] = {.process = lfrfid_worker_mode_read_process},
    [LFRFIDWorkerWrite] = {.process = lfrfid_worker_mode_write_process},
    [LFRFIDWorkerEmulate] = {.process = lfrfid_worker_mode_emulate_process},
    [LFRFIDWorkerReadRaw] = {.process = lfrfid_worker_mode_read_raw_process},
    [LFRFIDWorkerEmulateRaw] = {.process = lfrfid_worker_mode_emulate_raw_process},
};