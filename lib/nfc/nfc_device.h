/**
 * @file nfc_device.h
 * @brief Abstract interface for managing NFC device data.
 *
 * Under the hood, it makes use of the protocol-specific functions that each one of them provides
 * and abstracts it with a protocol-independent API.
 *
 * It does not perform any signal processing, but merely serves as a container with some handy
 * operations such as loading and saving from and to a file.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "protocols/nfc_device_base.h"
#include "protocols/nfc_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NfcDevice opaque type definition.
 */
typedef struct NfcDevice NfcDevice;

/**
 * @brief Loading callback function signature.
 *
 * A function with such signature can be set as a callback to indicate
 * the completion (or a failure) of nfc_device_load() and nfc_device_save() functions.
 *
 * This facility is commonly used to control GUI elements, such as progress dialogs.
 *
 * @param[in] context user-defined context that was passed in nfc_device_set_loading_callback().
 * @param[in] state true if the data was loaded successfully, false otherwise.
 */
typedef void (*NfcLoadingCallback)(void* context, bool state);

/**
 * @brief Allocate an NfcDevice instance.
 *
 * A newly created instance does not hold any data and thus is considered invalid. The most common
 * use case would be to set its data by calling nfc_device_set_data() right afterwards.
 *
 * @returns pointer to the allocated instance.
 */
NfcDevice* nfc_device_alloc(void);

/**
 * @brief Delete an NfcDevice instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void nfc_device_free(NfcDevice* instance);

/**
 * @brief Clear an NfcDevice instance.
 *
 * All data contained in the instance will be deleted and the instance itself will become invalid
 * as if it was just allocated.
 *
 * @param[in,out] instance pointer to the instance to be cleared.
 */
void nfc_device_clear(NfcDevice* instance);

/**
 * @brief Reset an NfcDevice instance.
 *
 * The data contained in the instance will be reset according to the protocol-defined procedure.
 * Unlike the nfc_device_clear() function, the instance will remain valid.
 *
 * @param[in,out] instance pointer to the instance to be reset.
 */
void nfc_device_reset(NfcDevice* instance);

/**
 * @brief Get the protocol identifier from an NfcDevice instance.
 *
 * If the instance is invalid, the return value will be NfcProtocolInvalid.
 *
 * @param[in] instance pointer to the instance to be queried.
 * @returns protocol identifier contained in the instance.
 */
NfcProtocol nfc_device_get_protocol(const NfcDevice* instance);

/**
 * @brief Get the protocol-specific data from an NfcDevice instance.
 *
 * The protocol parameter's behaviour is a bit tricky. The function will check
 * whether there is such a protocol somewhere in the protocol hierarchy and return
 * the data exactly from that level.
 *
 * Example: Call nfc_device_get_data() on an instance with Mf DESFire protocol.
 * The protocol hierarchy will look like the following:
 *
 * `Mf DESFire --> ISO14443-4A --> ISO14443-3A`
 *
 * Thus, the following values of the protocol parameter are valid:
 *
 *  * NfcProtocolIso14443_3a
 *  * NfcProtocolIso14443_4a
 *  * NfcProtocolMfDesfire
 *
 * and passing them to the call would result in the respective data being returned.
 *
 * However, supplying a protocol identifier which is not in the hierarchy will
 * result in a crash. This is to improve type safety.
 *
 * @param instance pointer to the instance to be queried
 * @param protocol protocol identifier of the data to be retrieved.
 * @returns pointer to the instance's data.
 */
const NfcDeviceData* nfc_device_get_data(const NfcDevice* instance, NfcProtocol protocol);

/**
 * @brief Get the protocol name by its identifier.
 *
 * This function does not require an instance as its return result depends only
 * the protocol identifier.
 *
 * @param[in] protocol numeric identifier of the protocol in question.
 * @returns pointer to a statically allocated string containing the protocol name.
 */
const char* nfc_device_get_protocol_name(NfcProtocol protocol);

/**
 * @brief Get the name of an NfcDevice instance.
 *
 * The return value may change depending on the instance's internal state and the name_type parameter.
 *
 * @param[in] instance pointer to the instance to be queried.
 * @param[in] name_type type of the name to be displayed.
 * @returns pointer to a statically allocated string containing the device name.
 */
const char* nfc_device_get_name(const NfcDevice* instance, NfcDeviceNameType name_type);

/**
 * @brief Get the unique identifier (UID) of an NfcDevice instance.
 *
 * The UID length is protocol-dependent. Additionally, a particular protocol might support
 * several UID lengths.
 *
 * @param[in] instance pointer to the instance to be queried.
 * @param[out] uid_len pointer to the variable to contain the UID length.
 * @returns pointer to the byte array containing the instance's UID.
 */
const uint8_t* nfc_device_get_uid(const NfcDevice* instance, size_t* uid_len);

/**
 * @brief Set the unique identifier (UID) of an NfcDevice instance.
 *
 * The UID length must be supported by the instance's protocol.
 *
 * @param[in,out] instance pointer to the instance to be modified.
 * @param[in] uid pointer to the byte array containing the new UID.
 * @param[in] uid_len length of the UID.
 * @return true if the UID was valid and set, false otherwise.
 */
bool nfc_device_set_uid(NfcDevice* instance, const uint8_t* uid, size_t uid_len);

/**
 * @brief Set the data and protocol of an NfcDevice instance.
 *
 * Any data previously contained in the instance will be deleted.
 *
 * @param[in,out] instance pointer to the instance to be modified.
 * @param[in] protocol numeric identifier of the data's protocol.
 * @param[in] protocol_data pointer to the protocol-specific data.
 */
void nfc_device_set_data(
    NfcDevice* instance,
    NfcProtocol protocol,
    const NfcDeviceData* protocol_data);

/**
 * @brief Copy (export) the data contained in an NfcDevice instance to an outside NfcDeviceData instance.
 *
 * This function does the inverse of nfc_device_set_data().

 * The protocol identifier passed as the protocol parameter MUST match the one
 * stored in the instance, otherwise a crash will occur.
 * This is to improve type safety.
 *
 * @param[in] instance pointer to the instance to be copied from.
 * @param[in] protocol numeric identifier of the instance's protocol.
 * @param[out] protocol_data pointer to the destination data.
 */
void nfc_device_copy_data(
    const NfcDevice* instance,
    NfcProtocol protocol,
    NfcDeviceData* protocol_data);

/**
 * @brief Check whether an NfcDevice instance holds certain data.
 *
 * This function's behaviour is similar to nfc_device_is_equal(), with the difference
 * that it takes NfcProtocol and NfcDeviceData* instead of the second NfcDevice*.
 *
 * The following code snippets [1] and [2] are equivalent:
 *
 * [1]
 * ```c
 * bool is_equal = nfc_device_is_equal(device1, device2);
 * ```
 * [2]
 * ```c
 * NfcProtocol protocol = nfc_device_get_protocol(device2);
 * const NfcDeviceData* data = nfc_device_get_data(device2, protocol);
 * bool is_equal = nfc_device_is_equal_data(device1, protocol, data);
 * ```
 *
 * @param[in] instance pointer to the instance to be compared.
 * @param[in] protocol protocol identifier of the data to be compared.
 * @param[in] protocol_data pointer to the NFC device data to be compared.
 * @returns true if the instance is of the right type and the data matches, false otherwise.
 */
bool nfc_device_is_equal_data(
    const NfcDevice* instance,
    NfcProtocol protocol,
    const NfcDeviceData* protocol_data);

/**
 * @brief Compare two NfcDevice instances to determine whether they are equal.
 *
 * @param[in] instance pointer to the first instance to be compared.
 * @param[in] other pointer to the second instance to be compared.
 * @returns true if both instances are considered equal, false otherwise.
 */
bool nfc_device_is_equal(const NfcDevice* instance, const NfcDevice* other);

/**
 * @brief Set the loading callback function.
 *
 * @param[in,out] instance pointer to the instance to be modified.
 * @param[in] callback pointer to a function to be called when the load operation completes.
 * @param[in] context pointer to a user-specific context (will be passed to the callback).
 */
void nfc_device_set_loading_callback(
    NfcDevice* instance,
    NfcLoadingCallback callback,
    void* context);

/**
 * @brief Save NFC device data form an NfcDevice instance to a file.
 *
 * @param[in] instance pointer to the instance to be saved.
 * @param[in] path pointer to a character string with a full file path.
 * @returns true if the data was successfully saved, false otherwise.
 */
bool nfc_device_save(NfcDevice* instance, const char* path);

/**
 * @brief Load NFC device data to an NfcDevice instance from a file.
 *
 * @param[in,out] instance pointer to the instance to be loaded into.
 * @param[in] path pointer to a character string with a full file path.
 * @returns true if the data was successfully loaded, false otherwise.
 */
bool nfc_device_load(NfcDevice* instance, const char* path);

#ifdef __cplusplus
}
#endif
