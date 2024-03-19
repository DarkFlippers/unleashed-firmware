#pragma once

#include <infrared.h>
#include <furi_hal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TIMINGS_AMOUNT 1024U

/** Interface struct of infrared worker */
typedef struct InfraredWorker InfraredWorker;
/** Interface struct of received signal */
typedef struct InfraredWorkerSignal InfraredWorkerSignal;

typedef enum {
    InfraredWorkerGetSignalResponseNew, /** Signal, provided by callback is new and encoder should be reseted */
    InfraredWorkerGetSignalResponseSame, /** Signal, provided by callback is same. No encoder resetting. */
    InfraredWorkerGetSignalResponseStop, /** No more signals available. */
} InfraredWorkerGetSignalResponse;

/** Callback type for providing next signal to send. Should be used with
 * infrared_worker_make_decoded_signal() or infrared_worker_make_raw_signal()
 */
typedef InfraredWorkerGetSignalResponse (
    *InfraredWorkerGetSignalCallback)(void* context, InfraredWorker* instance);

/** Callback type for 'message is sent' event */
typedef void (*InfraredWorkerMessageSentCallback)(void* context);

/** Callback type to call by InfraredWorker thread when new signal is received */
typedef void (
    *InfraredWorkerReceivedSignalCallback)(void* context, InfraredWorkerSignal* received_signal);

/** Allocate InfraredWorker
 *
 * @return just created instance of InfraredWorker
 */
InfraredWorker* infrared_worker_alloc(void);

/** Free InfraredWorker
 *
 * @param[in]   instance - InfraredWorker instance
 */
void infrared_worker_free(InfraredWorker* instance);

/** Start InfraredWorker thread, initialise furi_hal, prepare all work.
 *
 * @param[in]   instance - InfraredWorker instance
 */
void infrared_worker_rx_start(InfraredWorker* instance);

/** Stop InfraredWorker thread, deinitialize furi_hal.
 *
 * @param[in]   instance - InfraredWorker instance
 */
void infrared_worker_rx_stop(InfraredWorker* instance);

/** Set received data callback InfraredWorker
 *
 * @param[in]   instance - InfraredWorker instance
 * @param[in]   context - context to pass to callbacks
 * @param[in]   callback - InfraredWorkerReceivedSignalCallback callback
 */
void infrared_worker_rx_set_received_signal_callback(
    InfraredWorker* instance,
    InfraredWorkerReceivedSignalCallback callback,
    void* context);

/** Enable blinking on receiving any signal on IR port.
 *
 * @param[in]   instance - instance of InfraredWorker
 * @param[in]   enable - true if you want to enable blinking
 *                       false otherwise
 */
void infrared_worker_rx_enable_blink_on_receiving(InfraredWorker* instance, bool enable);

/** Enable decoding of received infrared signals.
 *
 * @param[in]   instance - instance of InfraredWorker
 * @param[in]   enable - true if you want to enable decoding
 *                       false otherwise
 */
void infrared_worker_rx_enable_signal_decoding(InfraredWorker* instance, bool enable);

/** Clarify is received signal either decoded or raw
 *
 * @param[in]   signal - received signal
 * @return      true if signal is decoded, false if signal is raw
 */
bool infrared_worker_signal_is_decoded(const InfraredWorkerSignal* signal);

/** Start transmitting signal. Callback InfraredWorkerGetSignalCallback should be
 * set before this function is called, as it calls for it to fill buffer before
 * starting transmission.
 *
 * @param[in]   instance - InfraredWorker instance
 */
void infrared_worker_tx_start(InfraredWorker* instance);

/** Stop transmitting signal. Waits for end of current signal and stops transmission.
 *
 * @param[in]   instance - InfraredWorker instance
 */
void infrared_worker_tx_stop(InfraredWorker* instance);

/** Set callback for providing next signal to send
 *
 * @param[in]   instance - InfraredWorker instance
 * @param[in]   context - context to pass to callbacks
 * @param[in]   callback - InfraredWorkerGetSignalCallback callback
 */
void infrared_worker_tx_set_get_signal_callback(
    InfraredWorker* instance,
    InfraredWorkerGetSignalCallback callback,
    void* context);

/** Set callback for end of signal transmitting
 *
 * @param[in]   instance - InfraredWorker instance
 * @param[in]   context - context to pass to callbacks
 * @param[in]   callback - InfraredWorkerMessageSentCallback callback
 */
void infrared_worker_tx_set_signal_sent_callback(
    InfraredWorker* instance,
    InfraredWorkerMessageSentCallback callback,
    void* context);

/** Callback to pass to infrared_worker_tx_set_get_signal_callback() if signal
 * is steady and will not be changed between infrared_worker start and stop.
 * Before starting transmission, desired steady signal must be set with
 * infrared_worker_set_decoded_signal() or infrared_worker_set_raw_signal().
 *
 * This function should not be called directly.
 *
 * @param[in]   context - context
 * @param[out]  instance - InfraredWorker instance
 */
InfraredWorkerGetSignalResponse
    infrared_worker_tx_get_signal_steady_callback(void* context, InfraredWorker* instance);

/** Acquire raw signal from interface struct 'InfraredWorkerSignal'.
 * First, you have to ensure that signal is raw.
 *
 * @param[in]   signal - received signal
 * @param[out]  timings - pointer to array of timings
 * @param[out]  timings_cnt - pointer to amount of timings
 */
void infrared_worker_get_raw_signal(
    const InfraredWorkerSignal* signal,
    const uint32_t** timings,
    size_t* timings_cnt);

/** Acquire decoded message from interface struct 'InfraredWorkerSignal'.
 * First, you have to ensure that signal is decoded.
 *
 * @param[in]   signal - received signal
 * @return      decoded INFRARED message
 */
const InfraredMessage* infrared_worker_get_decoded_signal(const InfraredWorkerSignal* signal);

/** Set current decoded signal for InfraredWorker instance
 *
 * @param[out]  instance - InfraredWorker instance
 * @param[in]   message - decoded signal
 */
void infrared_worker_set_decoded_signal(InfraredWorker* instance, const InfraredMessage* message);

/** Set current raw signal for InfraredWorker instance
 *
 * @param[out]  instance - InfraredWorker instance
 * @param[in]   timings - array of raw timings
 * @param[in]   timings_cnt - size of array of raw timings
 * @param[in]   frequency - carrier frequency in Hertz
 * @param[in]   duty_cycle - carrier duty cycle (0.0 - 1.0)
 */
void infrared_worker_set_raw_signal(
    InfraredWorker* instance,
    const uint32_t* timings,
    size_t timings_cnt,
    uint32_t frequency,
    float duty_cycle);

#ifdef __cplusplus
}
#endif
