#pragma once

#include "slix.h"

#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SlixPoller opaque type definition.
 */
typedef struct SlixPoller SlixPoller;

/**
 * @brief Enumeration of possible Slix poller event types.
 */
typedef enum {
    SlixPollerEventTypeError, /**< An error occured while reading card. */
    SlixPollerEventTypePrivacyUnlockRequest, /**< Poller requests password to disable privacy mode. */
    SlixPollerEventTypeReady, /**< The card was successfully read by the poller. */
} SlixPollerEventType;

/**
 * @brief Slix poller privacy unlock context data.
 */
typedef struct {
    SlixPassword password; /**< Privacy password. */
    bool password_set; /**< Filed to indicate that password was set or not. */
} SlixPollerEventDataPrivacyUnlockContext;

/**
 * @brief Slixs poller event data.
 */
typedef union {
    SlixError error; /**< Error code indicating card reaing fail reason. */
    SlixPollerEventDataPrivacyUnlockContext privacy_password; /**< Privacy unlock event context. */
} SlixPollerEventData;

/**
 * @brief Slix poller event structure.
 *
 * Upon emission of an event, an instance of this struct will be passed to the callback.
 */
typedef struct {
    SlixPollerEventType type; /**< Type of emmitted event. */
    SlixPollerEventData* data; /**< Pointer to event specific data. */
} SlixPollerEvent;

/**
 * @brief Transmit and receive Slix frames in poller mode.
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
 * @return SlixErrorNone on success, an error code on failure.
 */
SlixError slix_poller_send_frame(
    SlixPoller* instance,
    const BitBuffer* tx_data,
    BitBuffer* rx_data,
    uint32_t fwt);

/**
 * @brief Send get nxp system info command and parse response.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the SlixSystemInfo structure to be filled.
 * @return SlixErrorNone on success, an error code on failure.
 */
SlixError slix_poller_get_nxp_system_info(SlixPoller* instance, SlixSystemInfo* data);

/**
 * @brief Read signature from card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the SlixSignature structure to be filled.
 * @return SlixErrorNone on success, an error code on failure.
 */
SlixError slix_poller_read_signature(SlixPoller* instance, SlixSignature* data);

/**
 * @brief Get random number from card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[out] data pointer to the SlixRandomNumber structure to be filled.
 * @return SlixErrorNone on success, an error code on failure.
 */
SlixError slix_poller_get_random_number(SlixPoller* instance, SlixRandomNumber* data);

/**
 * @brief Set password to card.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] type SlixPasswordType instance.
 * @param[in] password SlixPassword instance.
 * @param[in] random_number SlixRandomNumber instance.
 * @return SlixErrorNone on success, an error code on failure.
 */
SlixError slix_poller_set_password(
    SlixPoller* instance,
    SlixPasswordType type,
    SlixPassword password,
    SlixRandomNumber random_number);

#ifdef __cplusplus
}
#endif
