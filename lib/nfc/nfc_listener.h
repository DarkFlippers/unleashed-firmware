/**
 * @file nfc_listener.h
 * @brief NFC card emulation library.
 *
 * Once started, it will respond to supported commands from an NFC reader, thus imitating
 * (or emulating) an NFC card. The responses will depend on the data that was supplied to
 * the listener, so various card types and different cards of the same type can be emulated.
 *
 * It will also make any changes necessary to the emulated data in response to the
 * reader commands if the protocol supports it.
 *
 * When running, NfcListener will generate events that the calling code must handle
 * by providing a callback function. The events passed to the callback are protocol-specific
 * and may include errors, state changes, data reception, special function requests and more.
 */
#pragma once

#include <nfc/protocols/nfc_generic_event.h>
#include <nfc/protocols/nfc_device_base.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NfcListener opaque type definition.
 */
typedef struct NfcListener NfcListener;

/**
 * @brief Allocate an NfcListener instance.
 *
 * @param[in] nfc pointer to an Nfc instance.
 * @param[in] protocol identifier of the protocol to be used.
 * @param[in] data pointer to the data to use during emulation.
 * @returns pointer to an allocated instance.
 *
 * @see nfc.h
 */
NfcListener* nfc_listener_alloc(Nfc* nfc, NfcProtocol protocol, const NfcDeviceData* data);

/**
 * @brief Delete an NfcListener instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void nfc_listener_free(NfcListener* instance);

/**
 * @brief Start an NfcListener instance.
 *
 * The callback logic is protocol-specific, so it cannot be described here in detail.
 * However, the callback return value ALWAYS determines what the listener should do next:
 * to continue whatever it was doing prior to the callback run or to stop.
 *
 * @param[in,out] instance pointer to the instance to be started.
 * @param[in] callback pointer to a user-defined callback function which will receive events.
 * @param[in] context pointer to a user-specific context (will be passed to the callback).
 */
void nfc_listener_start(NfcListener* instance, NfcGenericCallback callback, void* context);

/**
 * @brief Stop an NfcListener instance.
 *
 * The emulation process can be stopped explicitly (the other way is via the callback return value).
 *
 * @param[in,out] instance pointer to the instance to be stopped.
 */
void nfc_listener_stop(NfcListener* instance);

/**
 * @brief Get the protocol identifier an NfcListener instance was created with.
 *
 * @param[in] instance pointer to the instance to be queried.
 * @returns identifier of the protocol used by the instance.
 */
NfcProtocol nfc_listener_get_protocol(const NfcListener* instance);

/**
 * @brief Get the data that was that was provided for emulation.
 *
 * The protocol identifier passed as the protocol parameter MUST match the one
 * stored in the instance, otherwise a crash will occur.
 * This is to improve type safety.
 *
 * @param[in] instance pointer to the instance to be queried.
 * @param[in] protocol assumed protocol identifier of the data to be retrieved.
 * @returns pointer to the NFC device data.
 */
const NfcDeviceData* nfc_listener_get_data(const NfcListener* instance, NfcProtocol protocol);

#ifdef __cplusplus
}
#endif
