/**
 * @file expansion_worker.h
 * @brief Expansion module handling thread wrapper.
 *
 * The worker is started each time an expansion module is detected
 * and handles all of the communication protocols. Likewise, it is stopped
 * upon module disconnection or communication error.
 *
 * @warning This file is a private implementation detail. Please do not attempt to use it in applications.
 */
#pragma once

#include <furi_hal_serial_types.h>

/**
 * @brief Expansion worker opaque type declaration.
 */
typedef struct ExpansionWorker ExpansionWorker;

/**
 * @brief Worker callback type.
 *
 * @see expansion_worker_set_callback()
 *
 * @param[in,out] context pointer to a user-defined object.
 */
typedef void (*ExpansionWorkerCallback)(void* context);

/**
 * @brief Create an expansion worker instance.
 *
 * @param[in] serial_id numerical identifier of the serial to be used by the worker.
 * @returns pointer to the created instance.
 */
ExpansionWorker* expansion_worker_alloc(FuriHalSerialId serial_id);

/**
 * @brief Delete an expansion worker instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void expansion_worker_free(ExpansionWorker* instance);

/**
 * @brief Set the module disconnect callback.
 *
 * The callback will be triggered upon worker stop EXCEPT
 * when it was stopped via an expansion_worker_stop() call.
 *
 * In other words, the callback will ONLY be triggered if the worker was
 * stopped due to the user disconnecting/resetting/powering down the module,
 * or due to some communication error.
 *
 * @param[in,out] instance pointer to the worker instance to be modified.
 * @param[in] callback pointer to the callback function to be called under the above conditions.
 * @param[in] context pointer to a user-defined object, will be passed as a parameter to the callback.
 */
void expansion_worker_set_callback(
    ExpansionWorker* instance,
    ExpansionWorkerCallback callback,
    void* context);

/**
 * @brief Start the expansion module worker.
 *
 * @param[in,out] instance pointer to the worker instance to be started.
 */
void expansion_worker_start(ExpansionWorker* instance);

/**
 * @brief Stop the expansion module worker.
 *
 * If the worker was stopped via this call (and not because of module disconnect/
 * protocol error), the callback will not be triggered.
 *
 * @param[in,out] instance pointer to the worker instance to be stopped.
 */
void expansion_worker_stop(ExpansionWorker* instance);
