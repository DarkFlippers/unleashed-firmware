/**
 * @file ibutton_worker.h
 * 
 * iButton worker
 */

#pragma once

#include "ibutton_key.h"
#include "ibutton_protocols.h"
#include "ibutton_write_targets.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    iButtonWorkerWriteOK,
    iButtonWorkerWriteSameKey,
    iButtonWorkerWriteNoDetect,
    iButtonWorkerWriteCannotWrite,
    iButtonWorkerWriteStartTarget, // a new blank type is being attempted (progress UI)
    iButtonWorkerWriteNoEnabledTarget, // nothing enabled that can write this key
} iButtonWorkerWriteResult;

typedef void (*iButtonWorkerReadCallback)(void* context);
typedef void (*iButtonWorkerWriteCallback)(void* context, iButtonWorkerWriteResult result);
typedef void (*iButtonWorkerEmulateCallback)(void* context, bool emulated);

typedef struct iButtonWorker iButtonWorker;

/**
 * Allocate ibutton worker
 * @return iButtonWorker* 
 */
iButtonWorker* ibutton_worker_alloc(iButtonProtocols* protocols);

/**
 * Limit which blank types a write may try. A worker starts at
 * ibutton_write_targets_default(), so honouring the user's setting is opt-in.
 * @param worker
 * @param mask mask of iButtonWriteTarget bits
 */
void ibutton_worker_set_write_targets(iButtonWorker* worker, iButtonWriteTargetMask mask);

/**
 * Name of the blank type last attempted, "" before the first one.
 * @param worker
 * @return a static string, never NULL
 */
const char* ibutton_worker_get_write_chip_name(iButtonWorker* worker);

/**
 * Free ibutton worker
 * @param worker 
 */
void ibutton_worker_free(iButtonWorker* worker);

/**
 * Start ibutton worker thread
 * @param worker 
 */
void ibutton_worker_start_thread(iButtonWorker* worker);

/**
 * Stop ibutton worker thread
 * @param worker 
 */
void ibutton_worker_stop_thread(iButtonWorker* worker);

/**
 * Set "read success" callback
 * @param worker 
 * @param callback 
 * @param context 
 */
void ibutton_worker_read_set_callback(
    iButtonWorker* worker,
    iButtonWorkerReadCallback callback,
    void* context);

/**
 * Start read mode
 * @param worker 
 * @param key 
 */
void ibutton_worker_read_start(iButtonWorker* worker, iButtonKey* key);

/**
 * Set "write event" callback
 * @param worker 
 * @param callback 
 * @param context 
 */
void ibutton_worker_write_set_callback(
    iButtonWorker* worker,
    iButtonWorkerWriteCallback callback,
    void* context);

/**
 * Start write blank mode
 * @param worker 
 * @param key 
 */
void ibutton_worker_write_id_start(iButtonWorker* worker, iButtonKey* key);

/**
 * Start write copy mode
 * @param worker
 * @param key
 */
void ibutton_worker_write_copy_start(iButtonWorker* worker, iButtonKey* key);

/**
 * Set "emulate success" callback
 * @param worker 
 * @param callback 
 * @param context 
 */
void ibutton_worker_emulate_set_callback(
    iButtonWorker* worker,
    iButtonWorkerEmulateCallback callback,
    void* context);

/**
 * Start emulate mode
 * @param worker 
 * @param key 
 */
void ibutton_worker_emulate_start(iButtonWorker* worker, iButtonKey* key);

/**
 * Stop all modes
 * @param worker 
 */
void ibutton_worker_stop(iButtonWorker* worker);

#ifdef __cplusplus
}
#endif
