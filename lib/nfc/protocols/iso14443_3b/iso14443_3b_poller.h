#pragma once

#include "iso14443_3b.h"
#include <lib/nfc/nfc.h>

#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Iso14443_3bPoller opaque type definition.
 */
typedef struct Iso14443_3bPoller Iso14443_3bPoller;

/**
 * @brief Enumeration of possible Iso14443_3b poller event types.
 */
typedef enum {
    Iso14443_3bPollerEventTypeError, /**< An error occured during activation procedure. */
    Iso14443_3bPollerEventTypeReady, /**< The card was activated by the poller. */
} Iso14443_3bPollerEventType;

/**
 * @brief Iso14443_3b poller event data.
 */
typedef union {
    Iso14443_3bError error; /**< Error code indicating card activation fail reason. */
} Iso14443_3bPollerEventData;

/**
 * @brief Iso14443_3b poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    Iso14443_3bPollerEventType type; /**< Type of emmitted event. */
    Iso14443_3bPollerEventData* data; /**< Pointer to event specific data. */
} Iso14443_3bPollerEvent;

/**
 * @brief Transmit and receive Iso14443_3b frames in poller mode.
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
 * @return Iso14443_3bErrorNone on success, an error code on failure.
 */
Iso14443_3bError iso14443_3b_poller_send_frame(
    Iso14443_3bPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer);

/**
 * @brief Perform collision resolution procedure.
 *
 * Must ONLY be used inside the callback function.
 *
 * Perfoms the collision resolution procedure as defined in Iso14443-3b. The data
 * field will be filled with Iso14443-3b data on success.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the Iso14443_3b data structure to be filled.
 * @return Iso14443_3bErrorNone on success, an error code on failure.
 */
Iso14443_3bError iso14443_3b_poller_activate(Iso14443_3bPoller* instance, Iso14443_3bData* data);

/**
 * @brief Send HALT command to the card.
 *
 * Must ONLY be used inside the callback function.
 *
 * Halts card and changes internal Iso14443_3bPoller state to Idle.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @return Iso14443_3bErrorNone on success, an error code on failure.
 */
Iso14443_3bError iso14443_3b_poller_halt(Iso14443_3bPoller* instance);

#ifdef __cplusplus
}
#endif
