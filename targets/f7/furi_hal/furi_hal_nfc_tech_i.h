/**
 * @file furi_hal_nfc_tech_i.h
 * @brief NFC HAL technology-related private definitions.
 *
 * This file is an implementation detail. It must not be included in
 * any public API-related headers.
 *
 * This file is to be changed in an unlikely event of adding support
 * for a new NFC technology.
 */
#pragma once

#include <furi_hal_nfc.h>
#include <furi_hal_spi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure the NFC chip for use with this technology.
 *
 * Used for init() and deinit() functions.
 *
 * @param[in,out] handle pointer to the NFC chip SPI handle.
 * @returns FuriHalNfcErrorNone on success, any other error code on failure.
 */
typedef FuriHalNfcError (*FuriHalNfcChipConfig)(FuriHalSpiBusHandle* handle);

/**
 * @brief Transmit data using technology-specific framing and timings.
 *
 * @param[in,out] handle pointer to the NFC chip SPI handle.
 * @param[in] tx_data pointer to a byte array containing the data to be transmitted.
 * @param[in] tx_bits transmit data size, in bits.
 * @returns FuriHalNfcErrorNone on success, any other error code on failure.
 */
typedef FuriHalNfcError (
    *FuriHalNfcTx)(FuriHalSpiBusHandle* handle, const uint8_t* tx_data, size_t tx_bits);

/**
 * @brief Receive data using technology-specific framing and timings.
 *
 * @param[in,out] handle pointer to the NFC chip SPI handle.
 * @param[out] rx_data pointer to a byte array to be filled with received data.
 * @param[in] rx_data_size maximum received data length, in bytes.
 * @param[out] rx_bits pointer to a variable to contain received data length, in bits.
 * @returns FuriHalNfcErrorNone on success, any other error code on failure.
 */
typedef FuriHalNfcError (*FuriHalNfcRx)(
    FuriHalSpiBusHandle* handle,
    uint8_t* rx_data,
    size_t rx_data_size,
    size_t* rx_bits);

/**
 * @brief Wait for an event using technology-specific method.
 *
 * @param[in] timeout_ms maximum time to wait, in milliseconds.
 * @return bitmask of occurred events.
 */
typedef FuriHalNfcEvent (*FuriHalNfcWaitEvent)(uint32_t timeout_ms);

/**
 * @brief Go to sleep in listener mode.
 *
 * Puts the passive target logic into Sleep (Halt) state.
 *
 * @param[in,out] handle pointer to the NFC chip SPI handle.
 * @returns FuriHalNfcErrorNone on success, any other error code on failure.
 */
typedef FuriHalNfcError (*FuriHalNfcSleep)(FuriHalSpiBusHandle* handle);

/**
 * @brief Go to idle in listener mode.
 *
 * Puts the passive target logic into Sense (Idle) state.
 *
 * @param[in,out] handle pointer to the NFC chip SPI handle.
 * @returns FuriHalNfcErrorNone on success, any other error code on failure.
 */
typedef FuriHalNfcError (*FuriHalNfcIdle)(FuriHalSpiBusHandle* handle);

/**
 * @brief Technology-specific compenstaion values for pollers.
 *
 * Timing compensations are needed due to execution delays not accounted for
 * in standards, they are usually found out experimentally.
 *
 * The compensation value will be subtracted from the respective timer running
 * time, so positive values shorten timeouts, and negative ones make them longer.
 */
typedef struct {
    int32_t fdt; /**< Frame delay time compensation, in carrier cycles. */
    int32_t fwt; /**< Frame wait time compensaton, in carrier cycles. */
} FuriHalNfcPollerCompensation;

/**
 * @brief Abstract technology-specific poller structure.
 */
typedef struct {
    FuriHalNfcPollerCompensation compensation; /**< Compensation values in poller mode. */
    FuriHalNfcChipConfig init; /**< Pointer to the init() function. */
    FuriHalNfcChipConfig deinit; /**< Pointer to the deinit() function. */
    FuriHalNfcWaitEvent wait_event; /**< Pointer to the wait_event() function. */
    FuriHalNfcTx tx; /**< Pointer to the tx() function. */
    FuriHalNfcRx rx; /**< Pointer to the rx() function. */
} FuriHalNfcTechPollerBase;

/**
 * @brief Technology-specific compenstaion values for listeners.
 *
 * Same considerations apply as with FuriHalNfcPollerCompensation.
 */
typedef struct {
    int32_t fdt; /**< Frame delay time compensation, in carrier cycles. */
} FuriHalNfcListenerCompensation;

/**
 * @brief Abstract technology-specific listener structure.
 *
 * If the listener operating mode is not supported for a particular
 * technology, fill this structure with zeroes.
 */
typedef struct {
    FuriHalNfcListenerCompensation compensation; /**< Compensation values in listener mode. */
    FuriHalNfcChipConfig init; /**< Pointer to the init() function. */
    FuriHalNfcChipConfig deinit; /**< Pointer to the deinit() function. */
    FuriHalNfcWaitEvent wait_event; /**< Pointer to the wait_event() function. */
    FuriHalNfcTx tx; /**< Pointer to the tx() function. */
    FuriHalNfcRx rx; /**< Pointer to the rx() function. */
    FuriHalNfcSleep sleep; /**< Pointer to the sleep() function. */
    FuriHalNfcIdle idle; /**< Pointer to the idle() function. */
} FuriHalNfcTechListenerBase;

/**
 * @brief Abstract NFC technology definition structure.
 *
 * Each concrete technology implementation must fill this structure
 * with its proper functions and constants.
 */
typedef struct {
    FuriHalNfcTechPollerBase poller; /**< Structure containing the poller definition. */
    FuriHalNfcTechListenerBase listener; /**< Structure containing the listener definition. */
} FuriHalNfcTechBase;

/** @brief Technology declaration for ISO14443 (Type A). */
extern const FuriHalNfcTechBase furi_hal_nfc_iso14443a;
/** @brief Technology declaration for ISO14443 (Type B). */
extern const FuriHalNfcTechBase furi_hal_nfc_iso14443b;
/** @brief Technology declaration for ISO15693. */
extern const FuriHalNfcTechBase furi_hal_nfc_iso15693;
/** @brief Technology declaration for FeliCa. */
extern const FuriHalNfcTechBase furi_hal_nfc_felica;
/* Declare new tehcnologies here. */

/**
 * @brief Array of pointers to every supported technology.
 *
 * This variable is defined in furi_hal_nfc.c. It will need to be modified
 * in case when a new technology is to be added.
 */
extern const FuriHalNfcTechBase* furi_hal_nfc_tech[];

#ifdef __cplusplus
}
#endif
