#pragma once

#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_poller.h>

#include "iso14443_4a.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Iso14443_4aPoller opaque type definition.
 */
typedef struct Iso14443_4aPoller Iso14443_4aPoller;

/**
 * @brief Enumeration of possible Iso14443_4a poller event types.
 */
typedef enum {
    Iso14443_4aPollerEventTypeError, /**< An error occured during activation procedure. */
    Iso14443_4aPollerEventTypeReady, /**< The card was activated by the poller. */
} Iso14443_4aPollerEventType;

/**
 * @brief Iso14443_4a poller event data.
 */
typedef union {
    Iso14443_4aError error; /**< Error code indicating card activation fail reason. */
} Iso14443_4aPollerEventData;

/**
 * @brief Iso14443_4a poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    Iso14443_4aPollerEventType type; /**< Type of emmitted event. */
    Iso14443_4aPollerEventData* data; /**< Pointer to event specific data. */
} Iso14443_4aPollerEvent;

/**
 * @brief Transmit and receive Iso14443_4a blocks in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer. The fwt parameter is calculated during activation procedure.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @return Iso14443_4aErrorNone on success, an error code on failure.
 */
Iso14443_4aError iso14443_4a_poller_send_block(
    Iso14443_4aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer);

/**
 * @brief Send HALT command to the card.
 *
 * Must ONLY be used inside the callback function.
 *
 * Halts card and changes internal Iso14443_4aPoller state to Idle.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @return Iso14443_4aErrorNone on success, an error code on failure.
 */
Iso14443_4aError iso14443_4a_poller_halt(Iso14443_4aPoller* instance);

/**
 * @brief Read Answer To Select (ATS) from the card.
 *
 * Must ONLY be used inside the callback function.
 *
 * Send Request Answer To Select (RATS) command to the card and parse the response.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the buffer to be filled with ATS data.
 * @return Iso14443_4aErrorNone on success, an error code on failure.
 */
Iso14443_4aError
    iso14443_4a_poller_read_ats(Iso14443_4aPoller* instance, Iso14443_4aAtsData* data);

#ifdef __cplusplus
}
#endif
