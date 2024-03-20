/**
 * @file furi_hal_serial.h
 * 
 * Serial HAL API
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "furi_hal_serial_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize Serial
 *
 * Configures GPIO, configures and enables transceiver.
 *
 * @param      handle  Serial handle
 * @param      baud    baud rate
 */
void furi_hal_serial_init(FuriHalSerialHandle* handle, uint32_t baud);

/** De-initialize Serial
 *
 * Configures GPIO to analog, clears callback and callback context, disables
 * hardware
 *
 * @param      handle  Serial handle
 */
void furi_hal_serial_deinit(FuriHalSerialHandle* handle);

/** Suspend operation
 *
 * Suspend hardware, settings and callbacks are preserved
 *
 * @param      handle  Serial handle
 */
void furi_hal_serial_suspend(FuriHalSerialHandle* handle);

/** Resume operation
 *
 * Resumes hardware from suspended state
 *
 * @param      handle  Serial handle
 */
void furi_hal_serial_resume(FuriHalSerialHandle* handle);

/**
 * @brief Determine whether a certain baud rate is supported
 *
 * @param      handle  Serial handle
 * @param      baud baud rate to be checked
 * @returns    true if baud rate is supported, false otherwise.
 */
bool furi_hal_serial_is_baud_rate_supported(FuriHalSerialHandle* handle, uint32_t baud);

/** Changes baud rate
 *
 * @param      handle  Serial handle
 * @param      baud    baud rate
 */
void furi_hal_serial_set_br(FuriHalSerialHandle* handle, uint32_t baud);

/** Transmits data in semi-blocking mode
 *
 * Fills transmission pipe with data, returns as soon as all bytes from buffer
 * are in the pipe.
 *
 * Real transmission will be completed later. Use
 * `furi_hal_serial_tx_wait_complete` to wait for completion if you need it.
 *
 * @param      handle       Serial handle
 * @param      buffer       data
 * @param      buffer_size  data size (in bytes)
 */
void furi_hal_serial_tx(FuriHalSerialHandle* handle, const uint8_t* buffer, size_t buffer_size);

/** Wait until transmission is completed
 *
 * Ensures that all data has been sent.
 *
 * @param      handle  Serial handle
 */
void furi_hal_serial_tx_wait_complete(FuriHalSerialHandle* handle);

/** Serial RX events */
typedef enum {
    FuriHalSerialRxEventData = (1 << 0), /**< Data: new data available */
    FuriHalSerialRxEventIdle = (1 << 1), /**< Idle: bus idle detected */
    FuriHalSerialRxEventFrameError = (1 << 2), /**< Framing Error: incorrect frame detected */
    FuriHalSerialRxEventNoiseError = (1 << 3), /**< Noise Error: noise on the line detected */
    FuriHalSerialRxEventOverrunError = (1 << 4), /**< Overrun Error: no space for received data */
} FuriHalSerialRxEvent;

/** Receive callback
 *
 * @warning    Callback will be called in interrupt context, ensure thread
 *             safety on your side.
 * @param      handle   Serial handle
 * @param      event    FuriHalSerialRxEvent
 * @param      context  Callback context provided earlier
 */
typedef void (*FuriHalSerialAsyncRxCallback)(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context);

/** Start and sets Serial Receive callback
 *
 * @warning    Callback will be called in interrupt context, ensure thread
 *             safety on your side
 *
 * @param      handle         Serial handle
 * @param      callback       callback pointer
 * @param      context        callback context
 * @param[in]  report_errors  report RX error
 */
void furi_hal_serial_async_rx_start(
    FuriHalSerialHandle* handle,
    FuriHalSerialAsyncRxCallback callback,
    void* context,
    bool report_errors);

/** Stop Serial Receive
 *
 * @param      handle    Serial handle
 */
void furi_hal_serial_async_rx_stop(FuriHalSerialHandle* handle);

/** Check if there is data available for reading
 *
 * @warning    This function must be called only from the callback
 *             FuriHalSerialAsyncRxCallback
 *
 * @param      handle    Serial handle
 * @return     true if data is available for reading, false otherwise
 */
bool furi_hal_serial_async_rx_available(FuriHalSerialHandle* handle);

/** Get data Serial receive
 *
 * @warning    This function must be called only from the callback
 *             FuriHalSerialAsyncRxCallback
 *
 * @param      handle  Serial handle
 *
 * @return     data
 */
uint8_t furi_hal_serial_async_rx(FuriHalSerialHandle* handle);

/* DMA based Serial API */

#define FURI_HAL_SERIAL_DMA_BUFFER_SIZE (256u)

/** Receive DMA callback
 *
 * @warning    DMA Callback will be called in interrupt context, ensure thread
 *             safety on your side.
 *
 * @param      handle    Serial handle
 * @param      event     FuriHalSerialDmaRxEvent
 * @param      data_len  Received data
 * @param      context   Callback context provided earlier
 */
typedef void (*FuriHalSerialDmaRxCallback)(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    size_t data_len,
    void* context);

/**
 * @brief Enable an input/output directon
 *
 * Takes over the respective pin by reconfiguring it to
 * the appropriate alternative function.
 *
 * @param     handle        Serial handle
 * @param     direction     Direction to enable
 */
void furi_hal_serial_enable_direction(
    FuriHalSerialHandle* handle,
    FuriHalSerialDirection direction);

/**
 * @brief Disable an input/output directon
 *
 * Releases the respective pin by reconfiguring it to
 * initial state, making possible its use for other purposes.
 *
 * @param     handle        Serial handle
 * @param     direction     Direction to disable
 */
void furi_hal_serial_disable_direction(
    FuriHalSerialHandle* handle,
    FuriHalSerialDirection direction);

/**
 * @brief Get the GPIO pin associated with a serial
 *
 * @param     handle        Serial handle
 * @param     direction     Direction to query
 * @returns   pointer to the respective pin instance
 */
const GpioPin*
    furi_hal_serial_get_gpio_pin(FuriHalSerialHandle* handle, FuriHalSerialDirection direction);

/** Start and sets Serial event callback receive DMA
 *
 * @param      handle         Serial handle
 * @param      callback       callback pointer
 * @param      context        callback context
 * @param[in]  report_errors  report RX error
 */
void furi_hal_serial_dma_rx_start(
    FuriHalSerialHandle* handle,
    FuriHalSerialDmaRxCallback callback,
    void* context,
    bool report_errors);

/** Stop Serial receive DMA
 *
 * @param      handle  Serial handle
 */
void furi_hal_serial_dma_rx_stop(FuriHalSerialHandle* handle);

/** Get data Serial receive DMA
 *
 * @warning    This function must be called only from the callback
 *             FuriHalSerialDmaRxCallback
 *
 * @param      handle  Serial handle
 * @param      data    pointer to data buffer
 * @param      len     get data size (in bytes)
 *
 * @return     size actual data receive (in bytes)
 */
size_t furi_hal_serial_dma_rx(FuriHalSerialHandle* handle, uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif
