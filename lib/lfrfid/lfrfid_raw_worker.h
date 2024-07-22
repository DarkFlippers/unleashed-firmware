#pragma once
#include <furi.h>
#include "lfrfid_worker.h"
#include <toolbox/protocols/protocol_dict.h>
#include "protocols/lfrfid_protocols.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LFRFIDRawWorker LFRFIDRawWorker;

/**
 * @brief Allocate a new LFRFIDRawWorker instance
 * 
 * @return LFRFIDRawWorker* 
 */
LFRFIDRawWorker* lfrfid_raw_worker_alloc(void);

/**
 * @brief Free a LFRFIDRawWorker instance
 * 
 * @param worker LFRFIDRawWorker instance
 */
void lfrfid_raw_worker_free(LFRFIDRawWorker* worker);

/**
 * @brief Start reading
 * 
 * @param worker LFRFIDRawWorker instance
 * @param file_path path where file will be saved
 * @param frequency HW frequency
 * @param duty_cycle HW duty cycle
 * @param callback callback for read event
 * @param context context for callback
 */
void lfrfid_raw_worker_start_read(
    LFRFIDRawWorker* worker,
    const char* file_path,
    float frequency,
    float duty_cycle,
    LFRFIDWorkerReadRawCallback callback,
    void* context);

/**
 * @brief Start emulate
 * 
 * @param worker LFRFIDRawWorker instance
 * @param file_path path to file that will be emulated
 * @param callback callback for emulate event
 * @param context context for callback
 */
void lfrfid_raw_worker_start_emulate(
    LFRFIDRawWorker* worker,
    const char* file_path,
    LFRFIDWorkerEmulateRawCallback callback,
    void* context);

/**
 * @brief Stop worker
 * 
 * @param worker 
 */
void lfrfid_raw_worker_stop(LFRFIDRawWorker* worker);

#ifdef __cplusplus
}
#endif
