/**
 * @file nfc_poller.h
 * @brief NFC card reading library.
 *
 * Once started, it will try to activate and read a card using the designated protocol,
 * which is usually obtained by creating and starting an NfcScanner first.
 *
 * @see nfc_scanner.h
 *
 * When running, NfcPoller will generate events that the calling code must handle
 * by providing a callback function. The events passed to the callback are protocol-specific
 * and may include errors, state changes, data reception, special function requests and more.
 *
 */
#pragma once

#include <nfc/protocols/nfc_generic_event.h>
#include <nfc/protocols/nfc_device_base.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NfcPoller opaque type definition.
 */
typedef struct NfcPoller NfcPoller;

/**
 * @brief Extended generic Nfc event type.
 *
 * An extended generic Nfc event contains protocol poller and it's parent protocol event data.
 * If protocol has no parent, then events are produced by Nfc instance.
 *
 * The parent_event_data field is protocol-specific and should be cast to the appropriate type before use.
 */
typedef struct {
    NfcGenericInstance* poller; /**< Pointer to the protocol poller. */
    NfcGenericEventData*
        parent_event_data /**< Pointer to the protocol's parent poller event data. */;
} NfcGenericEventEx;

/**
 * @brief Extended generic Nfc event callback type.
 *
 * A function of this type must be passed as the callback parameter upon extended start of a poller.
 *
 * @param [in] event Nfc  extended generic event, passed by value, complete with protocol type and data.
 * @param [in,out] context pointer to the user-specific context (set when starting a poller/listener instance).
 * @returns the command which the event producer must execute.
 */
typedef NfcCommand (*NfcGenericCallbackEx)(NfcGenericEventEx event, void* context);

/**
 * @brief Allocate an NfcPoller instance.
 *
 * @param[in] nfc pointer to an Nfc instance.
 * @param[in] protocol identifier of the protocol to be used.
 * @returns pointer to an allocated instance.
 *
 * @see nfc.h
 */
NfcPoller* nfc_poller_alloc(Nfc* nfc, NfcProtocol protocol);

/**
 * @brief Delete an NfcPoller instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void nfc_poller_free(NfcPoller* instance);

/**
 * @brief Start an NfcPoller instance.
 *
 * The callback logic is protocol-specific, so it cannot be described here in detail.
 * However, the callback return value ALWAYS determines what the poller should do next:
 * to continue whatever it was doing prior to the callback run or to stop.
 *
 * @param[in,out] instance pointer to the instance to be started.
 * @param[in] callback pointer to a user-defined callback function which will receive events.
 * @param[in] context pointer to a user-specific context (will be passed to the callback).
 */
void nfc_poller_start(NfcPoller* instance, NfcGenericCallback callback, void* context);

/**
 * @brief Start an NfcPoller instance in extended mode.
 *
 * When nfc poller is started in extended mode, callback will be called with parent protocol events
 * and protocol instance. This mode enables to make custom poller state machines.
 *
 * @param[in,out] instance pointer to the instance to be started.
 * @param[in] callback pointer to a user-defined callback function which will receive events.
 * @param[in] context pointer to a user-specific context (will be passed to the callback).
 */
void nfc_poller_start_ex(NfcPoller* instance, NfcGenericCallbackEx callback, void* context);

/**
 * @brief Stop an NfcPoller instance.
 *
 * The reading process can be stopped explicitly (the other way is via the callback return value).
 *
 * @param[in,out] instance pointer to the instance to be stopped.
 */
void nfc_poller_stop(NfcPoller* instance);

/**
 * @brief Detect whether there is a card supporting a particular protocol in the vicinity.
 *
 * The behaviour of this function is protocol-defined, in general, it will do whatever is
 * necessary to determine whether a card supporting the current protocol is in the vicinity
 * and whether it is functioning normally.
 *
 * It is used automatically inside NfcScanner, so there is usually no need
 * to call it explicitly.
 *
 * @see nfc_scanner.h
 *
 * @param[in,out] instance pointer to the instance to perform the detection with.
 * @returns true if a supported card was detected, false otherwise.
 */
bool nfc_poller_detect(NfcPoller* instance);

/**
 * @brief Get the protocol identifier an NfcPoller instance was created with.
 *
 * @param[in] instance pointer to the instance to be queried.
 * @returns identifier of the protocol used by the instance.
 */
NfcProtocol nfc_poller_get_protocol(const NfcPoller* instance);

/**
 * @brief Get the data that was that was gathered during the reading process.
 *
 * @param[in] instance pointer to the instance to be queried.
 * @returns pointer to the NFC device data.
 */
const NfcDeviceData* nfc_poller_get_data(const NfcPoller* instance);

#ifdef __cplusplus
}
#endif
