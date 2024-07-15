#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <ble/core/auto/ble_types.h>

/* Callback signature for getting characteristic data
 * Is called when characteristic is created to get max data length. Data ptr is NULL in this case
 *   The result is passed to aci_gatt_add_char as "Char_Value_Length"
 * For updates, called with a context - see flipper_gatt_characteristic_update
 * Returns true if *data ownership is transferred to the caller and will be freed */
typedef bool (
    *cbBleGattCharacteristicData)(const void* context, const uint8_t** data, uint16_t* data_len);

/* Used to specify the type of data for a characteristic - constant or callback-based */
typedef enum {
    FlipperGattCharacteristicDataFixed,
    FlipperGattCharacteristicDataCallback,
} BleGattCharacteristicDataType;

typedef struct {
    Char_Desc_Uuid_t uuid;
    struct {
        cbBleGattCharacteristicData fn;
        const void* context;
    } data_callback;
    uint8_t uuid_type;
    uint8_t max_length;
    uint8_t security_permissions;
    uint8_t access_permissions;
    uint8_t gatt_evt_mask;
    uint8_t is_variable;
} BleGattCharacteristicDescriptorParams;

/* Describes a single characteristic, providing data or callbacks to get data */
typedef struct {
    const char* name;
    BleGattCharacteristicDescriptorParams* descriptor_params;
    union {
        struct {
            const uint8_t* ptr;
            uint16_t length;
        } fixed;
        struct {
            cbBleGattCharacteristicData fn;
            const void* context;
        } callback;
    } data;
    Char_UUID_t uuid;
    // Some packed bitfields to save space
    BleGattCharacteristicDataType data_prop_type : 2;
    uint8_t is_variable                          : 2;
    uint8_t uuid_type                            : 2;
    uint8_t char_properties;
    uint8_t security_permissions;
    uint8_t gatt_evt_mask;
} BleGattCharacteristicParams;

_Static_assert(
    sizeof(BleGattCharacteristicParams) == 36,
    "BleGattCharacteristicParams size must be 36 bytes");

typedef struct {
    const BleGattCharacteristicParams* characteristic;
    uint16_t handle;
    uint16_t descriptor_handle;
} BleGattCharacteristicInstance;

/* Initialize a characteristic instance; copies the characteristic descriptor 
 * into the instance */
void ble_gatt_characteristic_init(
    uint16_t svc_handle,
    const BleGattCharacteristicParams* char_descriptor,
    BleGattCharacteristicInstance* char_instance);

/* Delete a characteristic instance; frees the copied characteristic
 * descriptor from the instance */
void ble_gatt_characteristic_delete(
    uint16_t svc_handle,
    BleGattCharacteristicInstance* char_instance);

/* Update a characteristic instance; if source==NULL, uses the data from 
 * the characteristic:
 *  - For fixed data, fixed.ptr is used as the source if source==NULL
 *  - For callback-based data, collback.context is passed as the context
 *    if source==NULL 
 */
bool ble_gatt_characteristic_update(
    uint16_t svc_handle,
    BleGattCharacteristicInstance* char_instance,
    const void* source);

bool ble_gatt_service_add(
    uint8_t Service_UUID_Type,
    const Service_UUID_t* Service_UUID,
    uint8_t Service_Type,
    uint8_t Max_Attribute_Records,
    uint16_t* Service_Handle);

bool ble_gatt_service_delete(uint16_t svc_handle);

#ifdef __cplusplus
}
#endif
