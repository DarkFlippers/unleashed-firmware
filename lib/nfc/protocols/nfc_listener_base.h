/**
 * @file nfc_listener_base.h
 * @brief Abstract interface definitions for the NFC listener system.
 *
 * This file is an implementation detail. It must not be included in
 * any public API-related headers.
 *
 * @see nfc_listener.h
 *
 */
#pragma once

#include "nfc_generic_event.h"
#include "nfc_device_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocate a protocol-specific listener instance.
 *
 * For base listeners pass a pointer to an instance of type Nfc
 * as the base_listener parameter, otherwise it must be a pointer to another listener instance
 * (compare iso14443_3a/iso14443_3a_listener.c and iso14443_4a/iso14443_4a_listener.c).
 *
 * @see nfc_protocol.c
 *
 * The NFC device data passed as the data parameter is copied to the instance and may
 * change during the emulation in response to reader commands.
 *
 * To retrieve the modified data, NfcListenerGetData gets called by the NfcListener
 * implementation when the user code calls nfc_listener_get_data().
 *
 * @param[in] base_listener pointer to the parent listener instance.
 * @param[in] data pointer to the protocol-specific data to use during emulation.
 * @returns pointer to the allocated listener instance.
 */
typedef NfcGenericInstance* (
    *NfcListenerAlloc)(NfcGenericInstance* base_listener, NfcDeviceData* data);

/**
 * @brief Delete a protocol-specific listener instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
typedef void (*NfcListenerFree)(NfcGenericInstance* instance);

/**
 * @brief Set the callback function to handle events emitted by the listener instance.
 *
 * @see nfc_generic_event.h
 *
 * @param[in,out] listener
 * @param[in] callback pointer to the user-defined callback function which will receive events.
 * @param[in] context pointer to the user-specific context (will be passed to the callback).
 */
typedef void (*NfcListenerSetCallback)(
    NfcGenericInstance* listener,
    NfcGenericCallback callback,
    void* context);

/**
 * @brief Emulate a supported NFC card with given device data.
 *
 * @param[in] event protocol-specific event passed by the parent listener instance.
 * @param[in,out] context pointer to the protocol-specific listener instance.
 * @returns command to be executed by the parent listener instance.
 */
typedef NfcCommand (*NfcListenerRun)(NfcGenericEvent event, void* context);

/**
 * @brief Get the protocol-specific data that was that was provided for emulation.
 *
 * @param[in] instance pointer to the protocol-specific listener instance.
 * @returns pointer to the NFC device data.
 */
typedef const NfcDeviceData* (*NfcListenerGetData)(const NfcGenericInstance* instance);

/**
 * @brief Generic NFC listener interface.
 *
 * Each protocol must fill this structure with its own function implementations.
 * See above for the function signatures and descriptions.
 *
 * Additionally, see ${PROTOCOL_NAME}/${PROTOCOL_NAME}_listener.c for usage examples.
 */
typedef struct {
    NfcListenerAlloc alloc; /**< Pointer to the alloc() function. */
    NfcListenerFree free; /**< Pointer to the free() function. */
    NfcListenerSetCallback set_callback; /**< Pointer to the set_callback() function. */
    NfcListenerRun run; /**< Pointer to the run() function. */
    NfcListenerGetData get_data; /**< Pointer to the get_data() function. */
} NfcListenerBase;

#ifdef __cplusplus
}
#endif
