#pragma once

#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_listener.h>

#include "iso14443_4a.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso14443_4aListener Iso14443_4aListener;

typedef enum {
    Iso14443_4aListenerEventTypeHalted,
    Iso14443_4aListenerEventTypeFieldOff,
    Iso14443_4aListenerEventTypeReceivedData,
} Iso14443_4aListenerEventType;

typedef struct {
    BitBuffer* buffer;
} Iso14443_4aListenerEventData;

typedef struct {
    Iso14443_4aListenerEventType type;
    Iso14443_4aListenerEventData* data;
} Iso14443_4aListenerEvent;

/**
 * @brief Transmit Iso14443_4a blocks in listener mode.
 *
 * Must ONLY be used inside the callback function.
 *
 * @param[in, out] instance pointer to the instance to be used in the transaction.
 * @param[in] tx_buffer pointer to the buffer containing the data to be transmitted.
 * @return Iso14443_4aErrorNone on success, an error code on failure.
 */
Iso14443_4aError
    iso14443_4a_listener_send_block(Iso14443_4aListener* instance, const BitBuffer* tx_buffer);

#ifdef __cplusplus
}
#endif
