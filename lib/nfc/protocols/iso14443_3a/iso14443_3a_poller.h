#pragma once

#include "iso14443_3a.h"
#include <lib/nfc/nfc.h>

#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Iso14443_3aPoller opaque type definition.
 */
typedef struct Iso14443_3aPoller Iso14443_3aPoller;

/**
 * @brief Enumeration of possible Iso14443_3a poller event types.
 */
typedef enum {
    Iso14443_3aPollerEventTypeError, /**< An error occured during activation procedure. */
    Iso14443_3aPollerEventTypeReady, /**< The card was activated by the poller. */
} Iso14443_3aPollerEventType;

/**
 * @brief Iso14443_3a poller event data.
 */
typedef union {
    Iso14443_3aError error; /**< Error code indicating card activation fail reason. */
} Iso14443_3aPollerEventData;

/**
 * @brief Iso14443_3a poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    Iso14443_3aPollerEventType type; /**< Type of emmitted event. */
    Iso14443_3aPollerEventData* data; /**< Pointer to event specific data. */
} Iso14443_3aPollerEvent;

/**
 * @brief Transmit and receive Iso14443_3a frames in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer, with a timeout defined by the fwt parameter.
 *
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @return Iso14443_3aErrorNone on success, an error code on failure.
 */
Iso14443_3aError iso14443_3a_poller_txrx(
    Iso14443_3aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

/**
 * @brief Transmit and receive Iso14443_3a standard frames in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer, with a timeout defined by the fwt parameter.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @return Iso14443_3aErrorNone on success, an error code on failure.
 */
Iso14443_3aError iso14443_3a_poller_send_standard_frame(
    Iso14443_3aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

/**
 * @brief Transmit and receive Iso14443_3a frames with custom parity bits in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer, with a timeout defined by the fwt parameter.
 *
 * Custom parity bits must be set in the tx_buffer. The rx_buffer will contain
 * the received data with the parity bits.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @param[in] fwt frame wait time (response timeout), in carrier cycles.
 * @return Iso14443_3aErrorNone on success, an error code on failure.
 */
Iso14443_3aError iso14443_3a_poller_txrx_custom_parity(
    Iso14443_3aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

/**
 * @brief Checks presence of Iso14443_3a complient card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @return Iso14443_3aErrorNone if card is present, an error code otherwise.
 */
Iso14443_3aError iso14443_3a_poller_check_presence(Iso14443_3aPoller* instance);

/**
 * @brief Perform collision resolution procedure.
 *
 * Must ONLY be used inside the callback function.
 *
 * Perfoms the collision resolution procedure as defined in Iso14443-3a. The iso14443_3a_data
 * field will be filled with Iso14443-3a data on success.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] iso14443_3a_data pointer to the Iso14443_3a data structure to be filled.
 * @return Iso14443_3aErrorNone on success, an error code on failure.
 */
Iso14443_3aError
    iso14443_3a_poller_activate(Iso14443_3aPoller* instance, Iso14443_3aData* iso14443_3a_data);

/**
 * @brief Send HALT command to the card.
 *
 * Must ONLY be used inside the callback function.
 *
 * Halts card and changes internal Iso14443_3aPoller state to Idle.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @return Iso14443_3aErrorNone on success, an error code on failure.
 */
Iso14443_3aError iso14443_3a_poller_halt(Iso14443_3aPoller* instance);

#ifdef __cplusplus
}
#endif
