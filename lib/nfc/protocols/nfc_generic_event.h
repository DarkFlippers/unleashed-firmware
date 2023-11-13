/**
 * @file nfc_generic_event.h
 * @brief Generic Nfc stack event definitions.
 *
 * Events are the main way of passing information about, well, various events
 * that occur across the Nfc protocol stack.
 *
 * In order to subscribe to events from a certain instance, the user code must call
 * its corresponding start() function while providing the appropriate callback.
 * During this call, an additional context pointer can be provided, which will be passed
 * to the context parameter at the time of the callback execution.
 *
 * For additional information on how events are passed around and processed, see protocol-specific
 * poller and listener implementations found in the respectively named subfolders.
 *
 */
#pragma once

#include "nfc_protocol.h"
#include <nfc/nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generic Nfc instance type.
 *
 * Must be cast to a concrete type before use.
 * Depending on the context, a pointer of this type
 * may point to an object of the following types:
 * - Nfc type,
 * - Concrete poller type,
 * - Concrete listener type.
 */
typedef void NfcGenericInstance;

/**
 * @brief Generic Nfc event data type.
 *
 * Must be cast to a concrete type before use.
 * Usually, it will be the protocol-specific event type.
 */
typedef void NfcGenericEventData;

/**
 * @brief Generic Nfc event type.
 *
 * A generic Nfc event contains a protocol identifier, can be used to determine
 * the remaing fields' type.
 *
 * If the value of the protocol field is NfcProtocolInvalid, then it means that
 * the event was emitted from an Nfc instance, otherwise it originated from
 * a concrete poller or listener instance.
 *
 * The event_data field is protocol-specific and should be cast to the appropriate type before use.
 */
typedef struct {
    NfcProtocol protocol; /**< Protocol identifier of the instance that produced the event. */
    NfcGenericInstance*
        instance; /**< Pointer to the protocol-specific instance that produced the event. */
    NfcGenericEventData* event_data; /**< Pointer to the protocol-specific event. */
} NfcGenericEvent;

/**
 * @brief Generic Nfc event callback type.
 *
 * A function of this type must be passed as the callback parameter upon start
 * of a poller, listener or Nfc instance.
 *
 * @param [in] event Nfc generic event, passed by value, complete with protocol type and data.
 * @param [in,out] context pointer to the user-specific context (set when starting a poller/listener instance).
 * @returns the command which the event producer must execute.
 */
typedef NfcCommand (*NfcGenericCallback)(NfcGenericEvent event, void* context);

#ifdef __cplusplus
}
#endif
