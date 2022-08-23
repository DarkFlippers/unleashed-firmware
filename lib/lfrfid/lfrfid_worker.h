/**
 * @file lfrfid_worker.h
 * 
 * LFRFID worker
 */

#pragma once
#include <toolbox/protocols/protocol_dict.h>
#include "protocols/lfrfid_protocols.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LFRFIDWorkerWriteOK,
    LFRFIDWorkerWriteProtocolCannotBeWritten,
    LFRFIDWorkerWriteFobCannotBeWritten,
    LFRFIDWorkerWriteTooLongToWrite,
} LFRFIDWorkerWriteResult;

typedef enum {
    LFRFIDWorkerReadTypeAuto,
    LFRFIDWorkerReadTypeASKOnly,
    LFRFIDWorkerReadTypePSKOnly,
} LFRFIDWorkerReadType;

typedef enum {
    LFRFIDWorkerReadSenseStart, // TODO: not implemented
    LFRFIDWorkerReadSenseEnd, // TODO: not implemented
    LFRFIDWorkerReadSenseCardStart,
    LFRFIDWorkerReadSenseCardEnd,
    LFRFIDWorkerReadStartASK,
    LFRFIDWorkerReadStartPSK,
    LFRFIDWorkerReadDone,
} LFRFIDWorkerReadResult;

typedef enum {
    LFRFIDWorkerReadRawFileError,
    LFRFIDWorkerReadRawOverrun,
} LFRFIDWorkerReadRawResult;

typedef enum {
    LFRFIDWorkerEmulateRawFileError,
    LFRFIDWorkerEmulateRawOverrun,
} LFRFIDWorkerEmulateRawResult;

typedef void (
    *LFRFIDWorkerReadCallback)(LFRFIDWorkerReadResult result, ProtocolId protocol, void* context);
typedef void (*LFRFIDWorkerWriteCallback)(LFRFIDWorkerWriteResult result, void* context);

typedef void (*LFRFIDWorkerReadRawCallback)(LFRFIDWorkerReadRawResult result, void* context);
typedef void (*LFRFIDWorkerEmulateRawCallback)(LFRFIDWorkerEmulateRawResult result, void* context);

typedef struct LFRFIDWorker LFRFIDWorker;

/**
 * Allocate LF-RFID worker
 * @return LFRFIDWorker* 
 */
LFRFIDWorker* lfrfid_worker_alloc(ProtocolDict* dict);

/**
 * Free LF-RFID worker
 * @param worker 
 */
void lfrfid_worker_free(LFRFIDWorker* worker);

/**
 * Start LF-RFID worker thread
 * @param worker 
 */
void lfrfid_worker_start_thread(LFRFIDWorker* worker);

/**
 * Stop LF-RFID worker thread
 * @param worker 
 */
void lfrfid_worker_stop_thread(LFRFIDWorker* worker);

/**
 * @brief Start read mode
 * 
 * @param worker 
 * @param type 
 * @param callback 
 * @param context 
 */
void lfrfid_worker_read_start(
    LFRFIDWorker* worker,
    LFRFIDWorkerReadType type,
    LFRFIDWorkerReadCallback callback,
    void* context);

/**
 * @brief Start write mode
 * 
 * @param worker 
 * @param protocol 
 * @param callback 
 * @param context 
 */
void lfrfid_worker_write_start(
    LFRFIDWorker* worker,
    LFRFIDProtocol protocol,
    LFRFIDWorkerWriteCallback callback,
    void* context);

/**
 * Start emulate mode
 * @param worker 
 */
void lfrfid_worker_emulate_start(LFRFIDWorker* worker, LFRFIDProtocol protocol);

/**
 * @brief Start raw read mode
 * 
 * @param worker 
 * @param filename 
 * @param type 
 * @param callback 
 * @param context 
 */
void lfrfid_worker_read_raw_start(
    LFRFIDWorker* worker,
    const char* filename,
    LFRFIDWorkerReadType type,
    LFRFIDWorkerReadRawCallback callback,
    void* context);

/**
 * Emulate raw read mode
 * @param worker 
 * @param filename 
 * @param callback 
 * @param context 
 */
void lfrfid_worker_emulate_raw_start(
    LFRFIDWorker* worker,
    const char* filename,
    LFRFIDWorkerEmulateRawCallback callback,
    void* context);

/**
 * Stop all modes
 * @param worker 
 */
void lfrfid_worker_stop(LFRFIDWorker* worker);

#ifdef __cplusplus
}
#endif
