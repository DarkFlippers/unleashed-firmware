/** @file lfrfid_worker.h
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
    LFRFIDWorkerReadSenseStart, // TODO FL-3516: not implemented
    LFRFIDWorkerReadSenseEnd, // TODO FL-3516: not implemented
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

/** Allocate LF-RFID worker
 * @return LFRFIDWorker* 
 */
LFRFIDWorker* lfrfid_worker_alloc(ProtocolDict* dict);

/** Free LF-RFID worker
 *
 * @param      worker  The worker
 */
void lfrfid_worker_free(LFRFIDWorker* worker);

/** Start LF-RFID worker thread
 *
 * @param      worker  The worker
 */
void lfrfid_worker_start_thread(LFRFIDWorker* worker);

/** Stop LF-RFID worker thread
 *
 * @param      worker  The worker
 */
void lfrfid_worker_stop_thread(LFRFIDWorker* worker);

/** Start read mode
 *
 * @param      worker    The worker
 * @param      type      The type
 * @param      callback  The callback
 * @param      context   The context
 */
void lfrfid_worker_read_start(
    LFRFIDWorker* worker,
    LFRFIDWorkerReadType type,
    LFRFIDWorkerReadCallback callback,
    void* context);

/** Start write mode
 *
 * @param      worker    The worker
 * @param      protocol  The protocol
 * @param      callback  The callback
 * @param      context   The context
 */
void lfrfid_worker_write_start(
    LFRFIDWorker* worker,
    LFRFIDProtocol protocol,
    LFRFIDWorkerWriteCallback callback,
    void* context);

/** Start emulate mode
 *
 * @param      worker    The worker
 * @param[in]  protocol  The protocol
 */
void lfrfid_worker_emulate_start(LFRFIDWorker* worker, LFRFIDProtocol protocol);

/** Start raw read mode
 *
 * @param      worker    The worker
 * @param      filename  The filename
 * @param      type      The type
 * @param      callback  The callback
 * @param      context   The context
 */
void lfrfid_worker_read_raw_start(
    LFRFIDWorker* worker,
    const char* filename,
    LFRFIDWorkerReadType type,
    LFRFIDWorkerReadRawCallback callback,
    void* context);

/** Emulate raw read mode
 *
 * @param      worker    The worker
 * @param      filename  The filename
 * @param      callback  The callback
 * @param      context   The context
 */
void lfrfid_worker_emulate_raw_start(
    LFRFIDWorker* worker,
    const char* filename,
    LFRFIDWorkerEmulateRawCallback callback,
    void* context);

/** Stop all modes
 *
 * @param      worker  The worker
 */
void lfrfid_worker_stop(LFRFIDWorker* worker);

#ifdef __cplusplus
}
#endif
