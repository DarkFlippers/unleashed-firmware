/**
 * @file nfc_device_base_i.h
 * @brief Abstract interface definitions for the NFC device system.
 *
 * This file is an implementation detail. It must not be included in
 * any public API-related headers.
 */
#pragma once

#include "nfc_device_base.h"

#include <flipper_format.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocate the protocol-specific NFC device data instance.
 *
 * @returns pointer to the allocated instance.
 */
typedef NfcDeviceData* (*NfcDeviceAlloc)(void);

/**
 * @brief Delete the protocol-specific NFC device data instance.
 *
 * @param[in,out] data pointer to the instance to be deleted.
 */
typedef void (*NfcDeviceFree)(NfcDeviceData* data);

/**
 * @brief Reset the NFC device data instance.
 *
 * The behaviour is protocol-specific. Usually, required fields are zeroed or
 * set to their initial values.
 *
 * @param[in,out] data pointer to the instance to be reset.
 */
typedef void (*NfcDeviceReset)(NfcDeviceData* data);

/**
 * @brief Copy source instance's data into the destination so that they become equal.
 *
 * @param[in,out] data pointer to the destination instance.
 * @param[in] other pointer to the source instance.
 */
typedef void (*NfcDeviceCopy)(NfcDeviceData* data, const NfcDeviceData* other);

/**
 * @brief Deprecated. Do not use in new protocols.
 * @deprecated do not use in new protocols.
 *
 * @param[in,out] data pointer to the instance to be tested.
 * @param[in] device_type pointer to a FuriString containing a device type identifier.
 * @returns true if data was verified, false otherwise.
 */
typedef bool (*NfcDeviceVerify)(NfcDeviceData* data, const FuriString* device_type);

/**
 * @brief Load NFC device data from a FlipperFormat file.
 *
 * The FlipperFormat file structure must be initialised and open by the calling code.
 *
 * @param[in,out] data pointer to the instance to be loaded into.
 * @param[in] ff pointer to the FlipperFormat file instance.
 * @param[in] version file format version to use when loading.
 * @returns true if loaded successfully, false otherwise.
 */
typedef bool (*NfcDeviceLoad)(NfcDeviceData* data, FlipperFormat* ff, uint32_t version);

/**
 * @brief Save NFC device data to a FlipperFormat file.
 *
 * The FlipperFormat file structure must be initialised and open by the calling code.
 *
 * @param[in] data pointer to the instance to be saved.
 * @param[in] ff pointer to the FlipperFormat file instance.
 * @returns true if saved successfully, false otherwise.
 */
typedef bool (*NfcDeviceSave)(const NfcDeviceData* data, FlipperFormat* ff);

/**
 * @brief Compare two NFC device data instances.
 *
 * @param[in] data pointer to the first instance to be compared.
 * @param[in] other pointer to the second instance to be compared.
 * @returns true if instances are equal, false otherwise.
 */
typedef bool (*NfcDeviceEqual)(const NfcDeviceData* data, const NfcDeviceData* other);

/**
 * @brief Get a protocol-specific stateful NFC device name.
 *
 * The return value may change depending on the instance's internal state and the name_type parameter.
 *
 * @param[in] data pointer to the instance to be queried.
 * @param[in] name_type type of the name to be displayed.
 * @returns pointer to a statically allocated character string containing the appropriate name.
 */
typedef const char* (*NfcDeviceGetName)(const NfcDeviceData* data, NfcDeviceNameType name_type);

/**
 * @brief Get the NFC device's unique identifier (UID).
 *
 * The UID length is protocol-dependent. Additionally, a particular protocol might support
 * several UID lengths.
 *
 * @param[in] data pointer to the instance to be queried.
 * @param[out] uid_len pointer to the variable to contain the UID length.
 * @returns pointer to the byte array containing the device's UID.
 */
typedef const uint8_t* (*NfcDeviceGetUid)(const NfcDeviceData* data, size_t* uid_len);

/**
 * @brief Set the NFC device's unique identifier (UID).
 *
 * The UID length must be supported by the protocol in question.
 *
 * @param[in,out] data pointer to the instance to be modified.
 * @param[in] uid pointer to the byte array containing the new UID.
 * @param[in] uid_len length of the UID.
 * @return true if the UID was valid and set, false otherwise.
 */
typedef bool (*NfcDeviceSetUid)(NfcDeviceData* data, const uint8_t* uid, size_t uid_len);

/**
 * @brief Get the NFC device data associated with the parent protocol.
 *
 * The protocol the instance's data is associated with must have a parent.
 *
 * @param[in] data pointer to the instance to be queried.
 * @returns pointer to the data instance associated with the parent protocol.
 */
typedef NfcDeviceData* (*NfcDeviceGetBaseData)(const NfcDeviceData* data);

/**
 * @brief Generic NFC device interface.
 *
 * Each protocol must fill this structure with its own function implementations.
 */
typedef struct {
    const char*
        protocol_name; /**< Pointer to a statically-allocated string with the protocol name. */
    NfcDeviceAlloc alloc; /**< Pointer to the alloc() function. */
    NfcDeviceFree free; /**< Pointer to the free() function. */
    NfcDeviceReset reset; /**< Pointer to the reset() function. */
    NfcDeviceCopy copy; /**< Pointer to the copy() function. */
    NfcDeviceVerify verify; /**< Deprecated. Set to NULL in new protocols. */
    NfcDeviceLoad load; /**< Pointer to the load() function. */
    NfcDeviceSave save; /**< Pointer to the save() function. */
    NfcDeviceEqual is_equal; /**< Pointer to the is_equal() function. */
    NfcDeviceGetName get_name; /**< Pointer to the get_name() function. */
    NfcDeviceGetUid get_uid; /**< Pointer to the get_uid() function. */
    NfcDeviceSetUid set_uid; /**< Pointer to the set_uid() function. */
    NfcDeviceGetBaseData get_base_data; /**< Pointer to the get_base_data() function. */
} NfcDeviceBase;

#ifdef __cplusplus
}
#endif
