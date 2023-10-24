/**
 * @file nfc_device_i.h
 * @brief NfcDevice private types and definitions.
 *
 * This file is an implementation detail. It must not be included in
 * any public API-related headers.
 */
#pragma once

#include "nfc_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NfcDevice structure definition.
 */
struct NfcDevice {
    NfcProtocol protocol; /**< Numeric identifier of the data's protocol*/
    NfcDeviceData* protocol_data; /**< Pointer to the NFC device data. */

    NfcLoadingCallback
        loading_callback; /**< Pointer to the function to be called upon loading completion. */
    void* loading_callback_context; /**< Pointer to the context to be passed to the loading callback. */
};

/**
 * @brief Get the mutable (non-const) data from an NfcDevice instance.
 *
 * The behaviour is the same as with nfc_device_get_data(), but the
 * return pointer is non-const, allowing for changing data it is pointing to.
 *
 * @see nfc_device.h
 *
 * Under the hood, nfc_device_get_data() calls this and then adds const-ness to the return value.
 *
 * @param instance pointer to the instance to be queried
 * @param protocol protocol identifier of the data to be retrieved.
 * @returns pointer to the instance's (mutable) data.
 */
NfcDeviceData* nfc_device_get_data_ptr(const NfcDevice* instance, NfcProtocol protocol);

#ifdef __cplusplus
}
#endif
