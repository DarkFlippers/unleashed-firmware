#pragma once

#include <furi_hal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SubGhzTxRxWorkerCallbackHaveRead)(void* context);

typedef struct SubGhzTxRxWorker SubGhzTxRxWorker;

typedef enum {
    SubGhzTxRxWorkerStatusIDLE,
    SubGhzTxRxWorkerStatusTx,
    SubGhzTxRxWorkerStatusRx,
} SubGhzTxRxWorkerStatus;

/** 
 * SubGhzTxRxWorker, add data to transfer
 * @param instance  Pointer to a SubGhzTxRxWorker instance
 * @param data      *data
 * @param size      data size
 * @return bool     true if ok
 */
bool subghz_tx_rx_worker_write(SubGhzTxRxWorker* instance, uint8_t* data, size_t size);

/** 
 * SubGhzTxRxWorker, get available data
 * @param instance   Pointer to a SubGhzTxRxWorker instance
 * @return size_t    data size
 */
size_t subghz_tx_rx_worker_available(SubGhzTxRxWorker* instance);

/** 
 * SubGhzTxRxWorker, read data
 * @param instance   Pointer to a SubGhzTxRxWorker instance
 * @param data       *data
 * @param size       max data size, which can be read
 * @return size_t    data size, how much is actually read
 */
size_t subghz_tx_rx_worker_read(SubGhzTxRxWorker* instance, uint8_t* data, size_t size);

/** 
 * Сallback SubGhzTxRxWorker when there is data to read in an empty buffer
 * @param instance Pointer to a SubGhzTxRxWorker instance
 * @param callback SubGhzTxRxWorkerCallbackHaveRead callback
 * @param context
 */
void subghz_tx_rx_worker_set_callback_have_read(
    SubGhzTxRxWorker* instance,
    SubGhzTxRxWorkerCallbackHaveRead callback,
    void* context);

/** 
 * Allocate SubGhzTxRxWorker
 * @return SubGhzTxRxWorker* Pointer to a SubGhzTxRxWorker instance
 */
SubGhzTxRxWorker* subghz_tx_rx_worker_alloc();

/** 
 * Free SubGhzTxRxWorker
 * @param instance Pointer to a SubGhzTxRxWorker instance
 */
void subghz_tx_rx_worker_free(SubGhzTxRxWorker* instance);

/** 
 * Start SubGhzTxRxWorker
 * @param instance Pointer to a SubGhzTxRxWorker instance
 * @return bool - true if ok
 */
bool subghz_tx_rx_worker_start(SubGhzTxRxWorker* instance, uint32_t frequency);

/** 
 * Stop SubGhzTxRxWorker
 * @param instance Pointer to a SubGhzTxRxWorker instance
 */
void subghz_tx_rx_worker_stop(SubGhzTxRxWorker* instance);

/** 
 * Check if worker is running
 * @param instance Pointer to a SubGhzTxRxWorker instance
 * @return bool - true if running
 */
bool subghz_tx_rx_worker_is_running(SubGhzTxRxWorker* instance);

#ifdef __cplusplus
}
#endif
