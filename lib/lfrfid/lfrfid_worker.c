#include "lfrfid_worker_i.h"

#include <furi.h>
#include <furi_hal.h>

typedef enum {
    LFRFIDEventStopThread = (1 << 0),
    LFRFIDEventStopMode = (1 << 1),
    LFRFIDEventRead = (1 << 2),
    LFRFIDEventWrite = (1 << 3),
    LFRFIDEventEmulate = (1 << 4),
    LFRFIDEventReadRaw = (1 << 5),
    LFRFIDEventEmulateRaw = (1 << 6),
    LFRFIDEventAll =
        (LFRFIDEventStopThread | LFRFIDEventStopMode | LFRFIDEventRead | LFRFIDEventWrite |
         LFRFIDEventEmulate | LFRFIDEventReadRaw | LFRFIDEventEmulateRaw),
} LFRFIDEventType;

static int32_t lfrfid_worker_thread(void* thread_context);

LFRFIDWorker* lfrfid_worker_alloc(ProtocolDict* dict) {
    furi_check(dict);

    LFRFIDWorker* worker = malloc(sizeof(LFRFIDWorker));
    worker->mode_index = LFRFIDWorkerIdle;
    worker->read_cb = NULL;
    worker->write_cb = NULL;
    worker->cb_ctx = NULL;
    worker->raw_filename = NULL;
    worker->mode_storage = NULL;

    worker->thread = furi_thread_alloc_ex("LfrfidWorker", 2048, lfrfid_worker_thread, worker);

    worker->protocols = dict;

    return worker;
}

void lfrfid_worker_free(LFRFIDWorker* worker) {
    furi_check(worker);

    if(worker->raw_filename) {
        free(worker->raw_filename);
    }

    furi_thread_free(worker->thread);
    free(worker);
}

void lfrfid_worker_read_start(
    LFRFIDWorker* worker,
    LFRFIDWorkerReadType type,
    LFRFIDWorkerReadCallback callback,
    void* context) {
    furi_check(worker);
    furi_check(worker->mode_index == LFRFIDWorkerIdle);

    worker->read_type = type;
    worker->read_cb = callback;
    worker->cb_ctx = context;
    furi_thread_flags_set(furi_thread_get_id(worker->thread), LFRFIDEventRead);
}

void lfrfid_worker_write_start(
    LFRFIDWorker* worker,
    LFRFIDProtocol protocol,
    LFRFIDWorkerWriteCallback callback,
    void* context) {
    furi_check(worker->mode_index == LFRFIDWorkerIdle);
    worker->protocol = protocol;
    worker->write_cb = callback;
    worker->cb_ctx = context;
    furi_thread_flags_set(furi_thread_get_id(worker->thread), LFRFIDEventWrite);
}

void lfrfid_worker_emulate_start(LFRFIDWorker* worker, LFRFIDProtocol protocol) {
    furi_check(worker);
    furi_check(worker->mode_index == LFRFIDWorkerIdle);

    worker->protocol = protocol;
    furi_thread_flags_set(furi_thread_get_id(worker->thread), LFRFIDEventEmulate);
}

void lfrfid_worker_set_filename(LFRFIDWorker* worker, const char* filename) {
    if(worker->raw_filename) {
        free(worker->raw_filename);
    }

    worker->raw_filename = strdup(filename);
}

void lfrfid_worker_read_raw_start(
    LFRFIDWorker* worker,
    const char* filename,
    LFRFIDWorkerReadType type,
    LFRFIDWorkerReadRawCallback callback,
    void* context) {
    furi_check(worker);
    furi_check(worker->mode_index == LFRFIDWorkerIdle);

    worker->read_type = type;
    worker->read_raw_cb = callback;
    worker->cb_ctx = context;
    lfrfid_worker_set_filename(worker, filename);
    furi_thread_flags_set(furi_thread_get_id(worker->thread), LFRFIDEventReadRaw);
}

void lfrfid_worker_emulate_raw_start(
    LFRFIDWorker* worker,
    const char* filename,
    LFRFIDWorkerEmulateRawCallback callback,
    void* context) {
    furi_check(worker);
    furi_check(worker->mode_index == LFRFIDWorkerIdle);

    lfrfid_worker_set_filename(worker, filename);
    worker->emulate_raw_cb = callback;
    worker->cb_ctx = context;
    furi_thread_flags_set(furi_thread_get_id(worker->thread), LFRFIDEventEmulateRaw);
}

void lfrfid_worker_stop(LFRFIDWorker* worker) {
    furi_check(worker);

    furi_thread_flags_set(furi_thread_get_id(worker->thread), LFRFIDEventStopMode);
}

void lfrfid_worker_start_thread(LFRFIDWorker* worker) {
    furi_check(worker);

    furi_thread_start(worker->thread);
}

void lfrfid_worker_stop_thread(LFRFIDWorker* worker) {
    furi_check(worker);

    furi_thread_flags_set(furi_thread_get_id(worker->thread), LFRFIDEventStopThread);
    furi_thread_join(worker->thread);
}

bool lfrfid_worker_check_for_stop(LFRFIDWorker* worker) {
    UNUSED(worker);
    uint32_t flags = furi_thread_flags_get();
    return flags & LFRFIDEventStopMode;
}

size_t lfrfid_worker_dict_get_data_size(LFRFIDWorker* worker, LFRFIDProtocol protocol) {
    furi_assert(worker->mode_index == LFRFIDWorkerIdle);
    return protocol_dict_get_data_size(worker->protocols, protocol);
}

static int32_t lfrfid_worker_thread(void* thread_context) {
    LFRFIDWorker* worker = thread_context;

    while(true) {
        uint32_t flags = furi_thread_flags_wait(LFRFIDEventAll, FuriFlagWaitAny, FuriWaitForever);
        if(flags != (unsigned)FuriFlagErrorTimeout) {
            // stop thread
            if(flags & LFRFIDEventStopThread) break;

            // switch mode
            if(flags & LFRFIDEventRead) worker->mode_index = LFRFIDWorkerRead;
            if(flags & LFRFIDEventWrite) worker->mode_index = LFRFIDWorkerWrite;
            if(flags & LFRFIDEventEmulate) worker->mode_index = LFRFIDWorkerEmulate;
            if(flags & LFRFIDEventReadRaw) worker->mode_index = LFRFIDWorkerReadRaw;
            if(flags & LFRFIDEventEmulateRaw) worker->mode_index = LFRFIDWorkerEmulateRaw;

            // do mode, if it exists
            if(lfrfid_worker_modes[worker->mode_index].process) {
                lfrfid_worker_modes[worker->mode_index].process(worker);
            }

            // reset mode
            worker->mode_index = LFRFIDWorkerIdle;
        }
    }

    return 0;
}
