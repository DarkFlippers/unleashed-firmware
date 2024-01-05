#pragma once

#include <lib/nfc/protocols/iso14443_3b/iso14443_3b_poller.h>

#include "iso14443_4b.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Iso14443_4bPoller opaque type definition.
 */
typedef struct Iso14443_4bPoller Iso14443_4bPoller;

/**
 * @brief Enumeration of possible Iso14443_4b poller event types.
 */
typedef enum {
    Iso14443_4bPollerEventTypeError, /**< An error occured during activation procedure. */
    Iso14443_4bPollerEventTypeReady, /**< The card was activated by the poller. */
} Iso14443_4bPollerEventType;

/**
 * @brief Iso14443_4b poller event data.
 */
typedef union {
    Iso14443_4bError error; /**< Error code indicating card activation fail reason. */
} Iso14443_4bPollerEventData;

/**
 * @brief Iso14443_4b poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    Iso14443_4bPollerEventType type; /**< Type of emmitted event. */
    Iso14443_4bPollerEventData* data; /**< Pointer to event specific data. */
} Iso14443_4bPollerEvent;

/**
 * @brief Transmit and receive Iso14443_4b blocks in poller mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * The rx_buffer will be filled with any data received as a response to data
 * sent from tx_buffer. The fwt parameter is calculated during activation procedure.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @param[out] rx_buffer pointer to the buffer to be filled with received data.
 * @return Iso14443_4bErrorNone on success, an error code on failure.
 */
Iso14443_4bError iso14443_4b_poller_send_block(
    Iso14443_4bPoller* instance,
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
 * @return Iso14443_4bErrorNone on success, an error code on failure.
 */
Iso14443_4bError iso14443_4b_poller_halt(Iso14443_4bPoller* instance);

#ifdef __cplusplus
}
#endif
