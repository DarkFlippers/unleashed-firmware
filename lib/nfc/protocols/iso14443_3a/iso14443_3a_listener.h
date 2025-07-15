#pragma once

#include "iso14443_3a.h"
#include <nfc/nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso14443_3aListener Iso14443_3aListener;

typedef enum {
    Iso14443_3aListenerEventTypeFieldOff,
    Iso14443_3aListenerEventTypeHalted,

    Iso14443_3aListenerEventTypeReceivedStandardFrame,
    Iso14443_3aListenerEventTypeReceivedData,
} Iso14443_3aListenerEventType;

typedef struct {
    BitBuffer* buffer;
} Iso14443_3aListenerEventData;

typedef struct {
    Iso14443_3aListenerEventType type;
    Iso14443_3aListenerEventData* data;
} Iso14443_3aListenerEvent;

/**
 * @brief Transmit Iso14443_3a frames in listener mode.
 *
 * Must ONLY be used inside the callback function.
 *
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @return Iso14443_3aErrorNone on success, an error code on failure.
 */
Iso14443_3aError
    iso14443_3a_listener_tx(Iso14443_3aListener* instance, const BitBuffer* tx_buffer);

/**
 * @brief Transmit Iso14443_3a frames with custom parity bits in listener mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * Custom parity bits must be set in the tx_buffer.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @return Iso14443_3aErrorNone on success, an error code on failure.
 */
Iso14443_3aError iso14443_3a_listener_tx_with_custom_parity(
    Iso14443_3aListener* instance,
    const BitBuffer* tx_buffer);

/**
 * @brief Transmit Iso14443_3a standard frames in listener mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @return Iso14443_3aErrorNone on success, an error code on failure.
 */
Iso14443_3aError iso14443_3a_listener_send_standard_frame(
    Iso14443_3aListener* instance,
    const BitBuffer* tx_buffer);

#ifdef __cplusplus
}
#endif
