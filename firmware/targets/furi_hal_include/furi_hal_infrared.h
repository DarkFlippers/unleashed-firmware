/**
 * @file furi_hal_infrared.h
 * INFRARED HAL API
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INFRARED_MAX_FREQUENCY 56000
#define INFRARED_MIN_FREQUENCY 10000

typedef enum {
    FuriHalInfraredTxGetDataStateOk, /**< New data obtained */
    FuriHalInfraredTxGetDataStateDone, /**< New data obtained, and this is end of package */
    FuriHalInfraredTxGetDataStateLastDone, /**< New data obtained, and this is end of package and no more data available */
} FuriHalInfraredTxGetDataState;

/** Callback type for providing data to INFRARED DMA TX system. It is called every tim */
typedef FuriHalInfraredTxGetDataState (
    *FuriHalInfraredTxGetDataISRCallback)(void* context, uint32_t* duration, bool* level);

/** Callback type called every time signal is sent by DMA to Timer.
 *
 * Actually, it means there are 2 timings left to send for this signal, which is
 * almost end. Don't use this callback to stop transmission, as far as there are
 * next signal is charged for transmission by DMA.
 */
typedef void (*FuriHalInfraredTxSignalSentISRCallback)(void* context);

/** Signature of callback function for receiving continuous INFRARED rx signal.
 *
 * @param      ctx[in]       context to pass to callback
 * @param      level[in]     level of input INFRARED rx signal
 * @param      duration[in]  duration of continuous rx signal level in us
 */
typedef void (*FuriHalInfraredRxCaptureCallback)(void* ctx, bool level, uint32_t duration);

/** Signature of callback function for reaching silence timeout on INFRARED port.
 *
 * @param      ctx[in]  context to pass to callback
 */
typedef void (*FuriHalInfraredRxTimeoutCallback)(void* ctx);

/** Initialize INFRARED RX timer to receive interrupts.
 *
 * It provides interrupts for every RX-signal edge changing with its duration.
 */
void furi_hal_infrared_async_rx_start(void);

/** Deinitialize INFRARED RX interrupt.
 */
void furi_hal_infrared_async_rx_stop(void);

/** Setup hal for receiving silence timeout.
 *
 * Should be used with 'furi_hal_infrared_timeout_irq_set_callback()'.
 *
 * @param[in]  timeout_us  time to wait for silence on INFRARED port before
 *                         generating IRQ.
 */
void furi_hal_infrared_async_rx_set_timeout(uint32_t timeout_us);

/** Setup callback for previously initialized INFRARED RX interrupt.
 *
 * @param[in]  callback  callback to call when RX signal edge changing occurs
 * @param[in]  ctx       context for callback
 */
void furi_hal_infrared_async_rx_set_capture_isr_callback(
    FuriHalInfraredRxCaptureCallback callback,
    void* ctx);

/** Setup callback for reaching silence timeout on INFRARED port.
 *
 * Should setup hal with 'furi_hal_infrared_setup_rx_timeout_irq()' first.
 *
 * @param[in]  callback  callback for silence timeout
 * @param[in]  ctx       context to pass to callback
 */
void furi_hal_infrared_async_rx_set_timeout_isr_callback(
    FuriHalInfraredRxTimeoutCallback callback,
    void* ctx);

/** Check if INFRARED is in use now.
 *
 * @return     true if INFRARED is busy, false otherwise.
 */
bool furi_hal_infrared_is_busy(void);

/** Set callback providing new data.
 *
 * This function has to be called before furi_hal_infrared_async_tx_start().
 *
 * @param[in]  callback  function to provide new data
 * @param[in]  context   context for callback
 */
void furi_hal_infrared_async_tx_set_data_isr_callback(
    FuriHalInfraredTxGetDataISRCallback callback,
    void* context);

/** Start IR asynchronous transmission.
 *
 * It can be stopped by 2 reasons:
 * 1. implicit call for furi_hal_infrared_async_tx_stop()
 * 2. callback can provide FuriHalInfraredTxGetDataStateLastDone response which
 *    means no more data available for transmission.
 *
 * Any func (furi_hal_infrared_async_tx_stop() or
 * furi_hal_infrared_async_tx_wait_termination()) has to be called to wait end of
 * transmission and free resources.
 *
 * @param[in]  freq        frequency for PWM
 * @param[in]  duty_cycle  duty cycle for PWM
 */
void furi_hal_infrared_async_tx_start(uint32_t freq, float duty_cycle);

/** Stop IR asynchronous transmission and free resources.
 *
 * Transmission will stop as soon as transmission reaches end of package
 * (FuriHalInfraredTxGetDataStateDone or FuriHalInfraredTxGetDataStateLastDone).
 */
void furi_hal_infrared_async_tx_stop(void);

/** Wait for end of IR asynchronous transmission and free resources.
 *
 * Transmission will stop as soon as transmission reaches end of transmission
 * (FuriHalInfraredTxGetDataStateLastDone).
 */
void furi_hal_infrared_async_tx_wait_termination(void);

/** Set callback for end of signal transmission
 *
 * @param[in]  callback  function to call when signal is sent
 * @param[in]  context   context for callback
 */
void furi_hal_infrared_async_tx_set_signal_sent_isr_callback(
    FuriHalInfraredTxSignalSentISRCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
