#pragma once

#include <irda.h>
#include <furi-hal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TIMINGS_AMOUNT                  512

/** Interface struct of irda worker */
typedef struct IrdaWorker IrdaWorker;
/** Interface struct of received signal */
typedef struct IrdaWorkerSignal IrdaWorkerSignal;

typedef enum {
    IrdaWorkerGetSignalResponseNew,     /** Signal, provided by callback is new and encoder should be reseted */
    IrdaWorkerGetSignalResponseSame,    /** Signal, provided by callback is same. No encoder resetting. */
    IrdaWorkerGetSignalResponseStop,    /** No more signals available. */
} IrdaWorkerGetSignalResponse;

/** Callback type for providing next signal to send. Should be used with
 * irda_worker_make_decoded_signal() or irda_worker_make_raw_signal()
 */
typedef IrdaWorkerGetSignalResponse (*IrdaWorkerGetSignalCallback)(void* context, IrdaWorker* instance);

/** Callback type for 'message is sent' event */
typedef void (*IrdaWorkerMessageSentCallback)(void* context);


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

/** Start IrdaWorker thread, initialise furi-hal, prepare all work.
 *
 * @param[in]   instance - IrdaWorker instance
 */
void irda_worker_rx_start(IrdaWorker* instance);

/** Stop IrdaWorker thread, deinitialize furi-hal.
 *
 * @param[in]   instance - IrdaWorker instance
 */
void irda_worker_rx_stop(IrdaWorker* instance);

/** Set received data callback IrdaWorker
 *
 * @param[in]   instance - IrdaWorker instance
 * @param[in]   context - context to pass to callbacks
 * @param[in]   callback - IrdaWorkerReceivedSignalCallback callback
 */
void irda_worker_rx_set_received_signal_callback(IrdaWorker* instance, IrdaWorkerReceivedSignalCallback callback, void* context);

/** Enable blinking on receiving any signal on IR port.
 *
 * @param[in]   instance - instance of IrdaWorker
 * @param[in]   enable - true if you want to enable blinking
 *                       false otherwise
 */
void irda_worker_rx_enable_blink_on_receiving(IrdaWorker* instance, bool enable);

/** Clarify is received signal either decoded or raw
 *
 * @param[in]   signal - received signal
 * @return      true if signal is decoded, false if signal is raw
 */
bool irda_worker_signal_is_decoded(const IrdaWorkerSignal* signal);

/** Start transmitting signal. Callback IrdaWorkerGetSignalCallback should be
 * set before this function is called, as it calls for it to fill buffer before
 * starting transmission.
 *
 * @param[in]   instance - IrdaWorker instance
 */
void irda_worker_tx_start(IrdaWorker* instance);

/** Stop transmitting signal. Waits for end of current signal and stops transmission.
 *
 * @param[in]   instance - IrdaWorker instance
 */
void irda_worker_tx_stop(IrdaWorker* instance);

/** Set callback for providing next signal to send
 *
 * @param[in]   instance - IrdaWorker instance
 * @param[in]   context - context to pass to callbacks
 * @param[in]   callback - IrdaWorkerGetSignalCallback callback
 */
void irda_worker_tx_set_get_signal_callback(IrdaWorker* instance, IrdaWorkerGetSignalCallback callback, void* context);

/** Set callback for end of signal transmitting
 *
 * @param[in]   instance - IrdaWorker instance
 * @param[in]   context - context to pass to callbacks
 * @param[in]   callback - IrdaWorkerMessageSentCallback callback
 */
void irda_worker_tx_set_signal_sent_callback(IrdaWorker* instance, IrdaWorkerMessageSentCallback callback, void* context);

/** Callback to pass to irda_worker_tx_set_get_signal_callback() if signal
 * is steady and will not be changed between irda_worker start and stop.
 * Before starting transmission, desired steady signal must be set with
 * irda_worker_make_decoded_signal() or irda_worker_make_raw_signal().
 *
 * This function should not be implicitly called.
 *
 * @param[in]   context - context
 * @param[out]  instance - IrdaWorker instance
 */
IrdaWorkerGetSignalResponse irda_worker_tx_get_signal_steady_callback(void* context, IrdaWorker* instance);

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
 * @return      decoded IRDA message
 */
const IrdaMessage* irda_worker_get_decoded_signal(const IrdaWorkerSignal* signal);

/** Set current decoded signal for IrdaWorker instance
 *
 * @param[out]  instance - IrdaWorker instance
 * @param[in]   message - decoded signal
 */
void irda_worker_set_decoded_signal(IrdaWorker* instance, const IrdaMessage* message);

/** Set current raw signal for IrdaWorker instance
 *
 * @param[out]  instance - IrdaWorker instance
 * @param[in]   timings - array of raw timings
 * @param[in]   timings_cnt - size of array of raw timings
 */
void irda_worker_set_raw_signal(IrdaWorker* instance, const uint32_t* timings, size_t timings_cnt);

#ifdef __cplusplus
}
#endif

