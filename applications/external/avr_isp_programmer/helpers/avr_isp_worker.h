#pragma once

#include <furi_hal.h>

typedef struct AvrIspWorker AvrIspWorker;

typedef void (*AvrIspWorkerCallback)(void* context, bool connect_usb);

/** Allocate AvrIspWorker
 * 
 * @param context AvrIsp* context
 * @return AvrIspWorker* 
 */
AvrIspWorker* avr_isp_worker_alloc(void* context);

/** Free AvrIspWorker
 * 
 * @param instance AvrIspWorker instance
 */
void avr_isp_worker_free(AvrIspWorker* instance);

/** Callback AvrIspWorker
 *
 * @param instance AvrIspWorker instance
 * @param callback AvrIspWorkerOverrunCallback callback
 * @param context
 */
void avr_isp_worker_set_callback(
    AvrIspWorker* instance,
    AvrIspWorkerCallback callback,
    void* context);

/** Start AvrIspWorker
 * 
 * @param instance AvrIspWorker instance
 */
void avr_isp_worker_start(AvrIspWorker* instance);

/** Stop AvrIspWorker
 * 
 * @param instance AvrIspWorker instance
 */
void avr_isp_worker_stop(AvrIspWorker* instance);

/** Check if worker is running
 * @param instance AvrIspWorker instance
 * @return bool - true if running
 */
bool avr_isp_worker_is_running(AvrIspWorker* instance);
