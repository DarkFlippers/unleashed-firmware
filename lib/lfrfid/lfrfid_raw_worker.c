#include <furi_hal_rfid.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/buffer_stream.h>
#include <toolbox/varint.h>
#include "lfrfid_raw_worker.h"
#include "lfrfid_raw_file.h"
#include "tools/varint_pair.h"

#define EMULATE_BUFFER_SIZE    1024
#define RFID_DATA_BUFFER_SIZE  2048
#define READ_DATA_BUFFER_COUNT 4

#define TAG_EMULATE "RawEmulate"

// emulate mode
typedef struct {
    size_t overrun_count;
    FuriStreamBuffer* stream;
} RfidEmulateCtx;

typedef struct {
    uint32_t emulate_buffer_arr[EMULATE_BUFFER_SIZE];
    uint32_t emulate_buffer_ccr[EMULATE_BUFFER_SIZE];
    RfidEmulateCtx ctx;
} LFRFIDRawWorkerEmulateData;

typedef enum {
    HalfTransfer,
    TransferComplete,
} LFRFIDRawEmulateDMAEvent;

// read mode
#define READ_TEMP_DATA_SIZE 10

typedef struct {
    BufferStream* stream;
    VarintPair* pair;
} LFRFIDRawWorkerReadData;

// main worker
struct LFRFIDRawWorker {
    FuriString* file_path;
    FuriThread* thread;
    FuriEventFlag* events;

    LFRFIDWorkerEmulateRawCallback emulate_callback;
    LFRFIDWorkerReadRawCallback read_callback;
    void* context;

    float frequency;
    float duty_cycle;
};

typedef enum {
    LFRFIDRawWorkerEventStop,
} LFRFIDRawWorkerEvent;

static int32_t lfrfid_raw_read_worker_thread(void* thread_context);
static int32_t lfrfid_raw_emulate_worker_thread(void* thread_context);

LFRFIDRawWorker* lfrfid_raw_worker_alloc(void) {
    LFRFIDRawWorker* worker = malloc(sizeof(LFRFIDRawWorker));

    worker->thread = furi_thread_alloc_ex("LfrfidRawWorker", 2048, NULL, worker);
    worker->events = furi_event_flag_alloc();
    worker->file_path = furi_string_alloc();

    return worker;
}

void lfrfid_raw_worker_free(LFRFIDRawWorker* worker) {
    furi_check(worker);

    furi_thread_free(worker->thread);
    furi_event_flag_free(worker->events);
    furi_string_free(worker->file_path);

    free(worker);
}

void lfrfid_raw_worker_start_read(
    LFRFIDRawWorker* worker,
    const char* file_path,
    float freq,
    float duty_cycle,
    LFRFIDWorkerReadRawCallback callback,
    void* context) {
    furi_check(worker);
    furi_check(file_path);
    furi_check(furi_thread_get_state(worker->thread) == FuriThreadStateStopped);

    furi_string_set(worker->file_path, file_path);

    worker->frequency = freq;
    worker->duty_cycle = duty_cycle;
    worker->read_callback = callback;
    worker->context = context;

    furi_thread_set_callback(worker->thread, lfrfid_raw_read_worker_thread);

    furi_thread_start(worker->thread);
}

void lfrfid_raw_worker_start_emulate(
    LFRFIDRawWorker* worker,
    const char* file_path,
    LFRFIDWorkerEmulateRawCallback callback,
    void* context) {
    furi_check(worker);
    furi_check(file_path);
    furi_check(furi_thread_get_state(worker->thread) == FuriThreadStateStopped);

    furi_string_set(worker->file_path, file_path);
    worker->emulate_callback = callback;
    worker->context = context;
    furi_thread_set_callback(worker->thread, lfrfid_raw_emulate_worker_thread);
    furi_thread_start(worker->thread);
}

void lfrfid_raw_worker_stop(LFRFIDRawWorker* worker) {
    furi_check(worker);

    worker->emulate_callback = NULL;
    worker->context = NULL;
    worker->read_callback = NULL;
    furi_event_flag_set(worker->events, 1 << LFRFIDRawWorkerEventStop);
    furi_thread_join(worker->thread);
}

static void lfrfid_raw_worker_capture(bool level, uint32_t duration, void* context) {
    LFRFIDRawWorkerReadData* ctx = context;

    bool need_to_send = varint_pair_pack(ctx->pair, level, duration);

    if(need_to_send) {
        buffer_stream_send_from_isr(
            ctx->stream, varint_pair_get_data(ctx->pair), varint_pair_get_size(ctx->pair));
        varint_pair_reset(ctx->pair);
    }
}

static int32_t lfrfid_raw_read_worker_thread(void* thread_context) {
    LFRFIDRawWorker* worker = (LFRFIDRawWorker*)thread_context;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    LFRFIDRawFile* file = lfrfid_raw_file_alloc(storage);
    const char* filename = furi_string_get_cstr(worker->file_path);
    bool file_valid = lfrfid_raw_file_open_write(file, filename);

    LFRFIDRawWorkerReadData* data = malloc(sizeof(LFRFIDRawWorkerReadData));

    data->stream = buffer_stream_alloc(RFID_DATA_BUFFER_SIZE, READ_DATA_BUFFER_COUNT);
    data->pair = varint_pair_alloc();

    if(file_valid) {
        // write header
        file_valid = lfrfid_raw_file_write_header(
            file, worker->frequency, worker->duty_cycle, RFID_DATA_BUFFER_SIZE);
    }

    if(file_valid) {
        // setup carrier
        furi_hal_rfid_tim_read_start(worker->frequency, worker->duty_cycle);

        // stabilize detector
        furi_delay_ms(1500);

        // start capture
        furi_hal_rfid_tim_read_capture_start(lfrfid_raw_worker_capture, data);

        while(1) {
            Buffer* buffer = buffer_stream_receive(data->stream, 100);

            if(buffer != NULL) {
                file_valid = lfrfid_raw_file_write_buffer(
                    file, buffer_get_data(buffer), buffer_get_size(buffer));
                buffer_reset(buffer);
            }

            if(!file_valid) {
                if(worker->read_callback != NULL) {
                    // message file_error to worker
                    worker->read_callback(LFRFIDWorkerReadRawFileError, worker->context);
                }
                break;
            }

            if(buffer_stream_get_overrun_count(data->stream) > 0 &&
               worker->read_callback != NULL) {
                // message overrun to worker
                worker->read_callback(LFRFIDWorkerReadRawOverrun, worker->context);
            }

            uint32_t flags = furi_event_flag_get(worker->events);
            if(FURI_BIT(flags, LFRFIDRawWorkerEventStop)) {
                break;
            }
        }

        furi_hal_rfid_tim_read_capture_stop();
        furi_hal_rfid_tim_read_stop();
    } else {
        if(worker->read_callback != NULL) {
            // message file_error to worker
            worker->read_callback(LFRFIDWorkerReadRawFileError, worker->context);
        }
    }

    if(!file_valid) {
        const uint32_t available_flags = (1 << LFRFIDRawWorkerEventStop);
        while(true) {
            uint32_t flags = furi_event_flag_wait(
                worker->events, available_flags, FuriFlagWaitAny, FuriWaitForever);

            if(FURI_BIT(flags, LFRFIDRawWorkerEventStop)) {
                break;
            }
        }
    }

    varint_pair_free(data->pair);
    buffer_stream_free(data->stream);
    lfrfid_raw_file_free(file);
    furi_record_close(RECORD_STORAGE);
    free(data);

    return 0;
}

static void rfid_emulate_dma_isr(bool half, void* context) {
    RfidEmulateCtx* ctx = context;

    uint32_t flag = half ? HalfTransfer : TransferComplete;
    size_t len = furi_stream_buffer_send(ctx->stream, &flag, sizeof(uint32_t), 0);
    if(len != sizeof(uint32_t)) {
        ctx->overrun_count++;
    }
}

static int32_t lfrfid_raw_emulate_worker_thread(void* thread_context) {
    LFRFIDRawWorker* worker = thread_context;

    bool file_valid = true;

    LFRFIDRawWorkerEmulateData* data = malloc(sizeof(LFRFIDRawWorkerEmulateData));

    Storage* storage = furi_record_open(RECORD_STORAGE);
    data->ctx.overrun_count = 0;
    data->ctx.stream = furi_stream_buffer_alloc(sizeof(uint32_t), sizeof(uint32_t));

    LFRFIDRawFile* file = lfrfid_raw_file_alloc(storage);

    do {
        file_valid = lfrfid_raw_file_open_read(file, furi_string_get_cstr(worker->file_path));
        if(!file_valid) break;
        file_valid = lfrfid_raw_file_read_header(file, &worker->frequency, &worker->duty_cycle);
        if(!file_valid) break;

        for(size_t i = 0; i < EMULATE_BUFFER_SIZE; i++) {
            file_valid = lfrfid_raw_file_read_pair(
                file, &data->emulate_buffer_arr[i], &data->emulate_buffer_ccr[i], NULL);
            if(!file_valid) break;
            data->emulate_buffer_arr[i] /= 8;
            data->emulate_buffer_arr[i] -= 1;
            data->emulate_buffer_ccr[i] /= 8;
        }
    } while(false);

    furi_hal_rfid_tim_emulate_dma_start(
        data->emulate_buffer_arr,
        data->emulate_buffer_ccr,
        EMULATE_BUFFER_SIZE,
        rfid_emulate_dma_isr,
        &data->ctx);

    if(!file_valid && worker->emulate_callback != NULL) {
        // message file_error to worker
        worker->emulate_callback(LFRFIDWorkerEmulateRawFileError, worker->context);
    }

    if(file_valid) {
        uint32_t flag = 0;

        while(true) {
            size_t size =
                furi_stream_buffer_receive(data->ctx.stream, &flag, sizeof(uint32_t), 100);

            if(size == sizeof(uint32_t)) {
                size_t start = 0;
                if(flag == TransferComplete) {
                    start = (EMULATE_BUFFER_SIZE / 2);
                }

                for(size_t i = 0; i < (EMULATE_BUFFER_SIZE / 2); i++) {
                    file_valid = lfrfid_raw_file_read_pair(
                        file,
                        &data->emulate_buffer_arr[start + i],
                        &data->emulate_buffer_ccr[start + i],
                        NULL);
                    if(!file_valid) break;
                    data->emulate_buffer_arr[i] /= 8;
                    data->emulate_buffer_arr[i] -= 1;
                    data->emulate_buffer_ccr[i] /= 8;
                }
            } else if(size != 0) {
                data->ctx.overrun_count++;
            }

            if(!file_valid) {
                if(worker->emulate_callback != NULL) {
                    // message file_error to worker
                    worker->emulate_callback(LFRFIDWorkerEmulateRawFileError, worker->context);
                }
                break;
            }

            if(data->ctx.overrun_count > 0 && worker->emulate_callback != NULL) {
                // message overrun to worker
                worker->emulate_callback(LFRFIDWorkerEmulateRawOverrun, worker->context);
            }

            uint32_t flags = furi_event_flag_get(worker->events);
            if(FURI_BIT(flags, LFRFIDRawWorkerEventStop)) {
                break;
            };
        }
    }

    furi_hal_rfid_tim_emulate_dma_stop();

    if(!file_valid) {
        const uint32_t available_flags = (1 << LFRFIDRawWorkerEventStop);
        while(true) {
            uint32_t flags = furi_event_flag_wait(
                worker->events, available_flags, FuriFlagWaitAny, FuriWaitForever);

            if(FURI_BIT(flags, LFRFIDRawWorkerEventStop)) {
                break;
            };
        }
    }

    if(data->ctx.overrun_count) {
        FURI_LOG_E(TAG_EMULATE, "overruns: %zu", data->ctx.overrun_count);
    }

    furi_stream_buffer_free(data->ctx.stream);
    lfrfid_raw_file_free(file);
    furi_record_close(RECORD_STORAGE);
    free(data);

    return 0;
}
