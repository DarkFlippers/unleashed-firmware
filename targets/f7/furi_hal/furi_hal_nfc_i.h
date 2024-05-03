/**
 * @file furi_hal_nfc_i.h
 * @brief NFC HAL library (private definitions).
 *
 * This file is an implementation detail. It must not be included in
 * any public API-related headers.
 */
#pragma once

#include <furi.h>
#include <furi_hal_nfc.h>
#include <furi_hal_spi.h>

#include <drivers/st25r3916.h>
#include <drivers/st25r3916_reg.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Common frame delay time compensation for pollers. */
#define FURI_HAL_NFC_POLLER_FDT_COMP_FC (-500)
/** @brief Common frame wait time compensation for pollers. */
#define FURI_HAL_NFC_POLLER_FWT_COMP_FC (FURI_HAL_NFC_POLLER_FDT_COMP_FC)

/**
 * @brief Enumeration containing bitmask values for NFC HAL internal events.
 */
typedef enum {
    FuriHalNfcEventInternalTypeAbort = (1U << 0), /**< Abort waiting for hardware events. */
    FuriHalNfcEventInternalTypeIrq = (1U << 1), /**< NFC hardware interrupt has occurred. */
    FuriHalNfcEventInternalTypeTimerFwtExpired =
        (1U << 2), /**< Frame wait time timeout has expired. */
    FuriHalNfcEventInternalTypeTimerBlockTxExpired =
        (1U << 3), /**< Transmission block timeout has expired. */
    FuriHalNfcEventInternalTypeTransparentDataReceived =
        (1U << 4), /**< Data was received in transparent mode. */
} FuriHalNfcEventInternalType;

/** @brief Special bitmask value of all internal events. */
#define FURI_HAL_NFC_EVENT_INTERNAL_ALL                                   \
    ((FuriHalNfcEventInternalTypeAbort | FuriHalNfcEventInternalTypeIrq | \
      FuriHalNfcEventInternalTypeTimerFwtExpired |                        \
      FuriHalNfcEventInternalTypeTimerBlockTxExpired |                    \
      FuriHalNfcEventInternalTypeTransparentDataReceived))

/**
 * @brief NFC HAL internal event structure.
 */
typedef struct {
    FuriThreadId thread; /**< Identifier of the thread that will be receiving events. */
    void* context; /**< Pointer to the user-provided context (will be passed to the event callback). */
} FuriHalNfcEventInternal;

/**
 * @brief NFC HAL global state structure.
 */
typedef struct {
    FuriMutex* mutex; /**< Pointer to the mutex serving as global NFC HAL lock. */
    FuriHalNfcMode mode; /**< Currently selected operating mode. */
    FuriHalNfcTech tech; /**< Currently selected NFC technology. */
} FuriHalNfc;

/**
 * @brief NFC HAL global state object declaration.
 */
extern FuriHalNfc furi_hal_nfc;

/**
 * @brief Initialise NFC HAL event system.
 */
void furi_hal_nfc_event_init(void);

/**
 * @brief Forcibly emit (a) particular internal event(s).
 *
 * @param[in] event bitmask of one or more events to be emitted.
 */
void furi_hal_nfc_event_set(FuriHalNfcEventInternalType event);

/**
 * @brief Initialise GPIO to generate an interrupt from the NFC hardware.
 */
void furi_hal_nfc_init_gpio_isr(void);

/**
 * @brief Disable interrupts from the NFC hardware.
 */
void furi_hal_nfc_deinit_gpio_isr(void);

/**
 * @brief Initialise all NFC timers.
 */
void furi_hal_nfc_timers_init(void);

/**
 * @brief Disable all NFC timers.
 */
void furi_hal_nfc_timers_deinit(void);

/**
 * @brief Get the interrupt bitmask from the NFC hardware.
 *
 * @param[in,out] handle pointer to the SPI handle associated with the NFC chip.
 * @returns bitmask of zero or more occurred interrupts.
 */
uint32_t furi_hal_nfc_get_irq(FuriHalSpiBusHandle* handle);

/**
 * @brief Wait until a specified type of interrupt occurs.
 *
 * @param[in,out] handle pointer to the SPI handle associated with the NFC chip.
 * @param[in] mask bitmask of one or more interrupts to wait for.
 * @param[in] timeout_ms maximum time to wait for an interrupt, in milliseconds.
 * @returns true if specified interrupt(s) have occured within timeout, false otherwise.
 */
bool furi_hal_nfc_event_wait_for_specific_irq(
    FuriHalSpiBusHandle* handle,
    uint32_t mask,
    uint32_t timeout_ms);

/**
 * @brief Wait for any event to occur.
 *
 * This function is common to all technologies.
 *
 * @param[in] timeout_ms maximum time to wait for an event, in milliseconds.
 * @returns bitmask of zero or more occurred events.
 */
FuriHalNfcEvent furi_hal_nfc_wait_event_common(uint32_t timeout_ms);

/**
 * @brief Start reception in listener mode.
 *
 * This function is common to all technologies.
 *
 * @param[in,out] handle pointer to the SPI handle associated with the NFC chip.
 * @returns FuriHalNfcErrorNone on success, any other error code on failure.
 */
FuriHalNfcError furi_hal_nfc_common_listener_rx_start(FuriHalSpiBusHandle* handle);

/**
 * @brief Transmit data using on-chip FIFO.
 *
 * This function is common to all technologies.
 *
 * @param[in,out] handle pointer to the SPI handle associated with the NFC chip.
 * @param[in] tx_data pointer to a byte array containing the data to be transmitted.
 * @param[in] tx_bits transmit data size, in bits.
 * @returns FuriHalNfcErrorNone on success, any other error code on failure.
 */
FuriHalNfcError furi_hal_nfc_common_fifo_tx(
    FuriHalSpiBusHandle* handle,
    const uint8_t* tx_data,
    size_t tx_bits);

/**
 * @brief Receive data using on-chip FIFO.
 *
 * This function is common to all technologies.
 *
 * @param[in,out] handle pointer to the SPI handle associated with the NFC chip.
 * @param[out] rx_data pointer to a byte array to be filled with received data.
 * @param[in] rx_data_size maximum received data size, in bytes.
 * @param[out] rx_bits pointer to the variable to contain the received data size, in bits.
 * @returns FuriHalNfcErrorNone on success, any other error code on failure.
 */
FuriHalNfcError furi_hal_nfc_common_fifo_rx(
    FuriHalSpiBusHandle* handle,
    uint8_t* rx_data,
    size_t rx_data_size,
    size_t* rx_bits);

/**
 * @brief Transmit data in poller mode.
 *
 * This function is common to all technologies.
 *
 * @param[in,out] handle pointer to the SPI handle associated with the NFC chip.
 * @param[in] tx_data pointer to a byte array containing the data to be transmitted.
 * @param[in] tx_bits transmit data size, in bits.
 * @returns FuriHalNfcErrorNone on success, any other error code on failure.
 */
FuriHalNfcError furi_hal_nfc_poller_tx_common(
    FuriHalSpiBusHandle* handle,
    const uint8_t* tx_data,
    size_t tx_bits);

#ifdef __cplusplus
}
#endif
