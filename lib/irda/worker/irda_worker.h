#pragma once

#include <irda.h>
#include <api-hal.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Interface struct of irda worker */
typedef struct IrdaWorker IrdaWorker;
/** Interface struct of received signal */
typedef struct IrdaWorkerSignal IrdaWorkerSignal;

/** Callback type to call by IrdaWorker thread when new signal is received */
typedef void (*IrdaWorkerReceivedSignalCallback)(void* context, IrdaWorkerSignal* received_signal);

/** Allocate IrdaWorker
 *
 * @return just created instance of IrdaWorker
 */
IrdaWorker* irda_worker_alloc();

/** Free IrdaWorker
 *
 * @param[in]   instance - IrdaWorker instance
 */
void irda_worker_free(IrdaWorker* instance);

/** Received data callback IrdaWorker
 *
 * @param[in]   instance - IrdaWorker instance
 * @param[in]   callback - IrdaWorkerReceivedSignalCallback callback
 */
void irda_worker_set_received_signal_callback(IrdaWorker* instance, IrdaWorkerReceivedSignalCallback callback);

/** Context callback IrdaWorker
 *
 * @param[in]   instance - IrdaWorker instance
 * @param[in]   context - context to pass to callbacks
 */
void irda_worker_set_context(IrdaWorker* instance, void* context);

/** Start IrdaWorker thread, initialise api-hal, prepare all work.
 *
 * @param[in]   instance - IrdaWorker instance
 */
void irda_worker_start(IrdaWorker* instance);

/** Stop IrdaWorker thread, deinitialize api-hal.
 *
 * @param[in]   instance - IrdaWorker instance
 */
void irda_worker_stop(IrdaWorker* instance);

/** Clarify is received signal either decoded or raw
 *
 * @param[in]   signal - received signal
 * @return      true if signal is decoded, false if signal is raw
 */
bool irda_worker_signal_is_decoded(const IrdaWorkerSignal* signal);

/** Acquire raw signal from interface struct 'IrdaWorkerSignal'.
 * First, you have to ensure that signal is raw.
 *
 * @param[in]   signal - received signal
 * @param[out]  timings - pointer to array of timings
 * @param[out]  timings_cnt - pointer to amount of timings
 */
void irda_worker_get_raw_signal(const IrdaWorkerSignal* signal, const uint32_t** timings, size_t* timings_cnt);

/** Acquire decoded message from interface struct 'IrdaWorkerSignal'.
 * First, you have to ensure that signal is decoded.
 *
 * @param[in]   signal - received signal
 * @return      decoded irda message
 */
const IrdaMessage* irda_worker_get_decoded_message(const IrdaWorkerSignal* signal);

/** Enable blinking on receiving any signal on IR port.
 *
 * @param[in]   instance - instance of IrdaWorker
 * @param[in]   enable - true if you want to enable blinking
 *                       false otherwise
 */
void irda_worker_enable_blink_on_receiving(IrdaWorker* instance, bool enable);

#ifdef __cplusplus
}
#endif

