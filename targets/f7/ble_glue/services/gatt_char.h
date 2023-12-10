#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <ble/ble.h>

#ifdef __cplusplus
extern "C" {
#endif

// Callback signature for getting characteristic data
// Is called when characteristic is created to get max data length. Data ptr is NULL in this case
//   The result is passed to aci_gatt_add_char as "Char_Value_Length"
// For updates, called with a context - see flipper_gatt_characteristic_update
// Returns true if *data ownership is transferred to the caller and will be freed
typedef bool (*cbFlipperGattCharacteristicData)(
    const void* context,
    const uint8_t** data,
    uint16_t* data_len);

typedef enum {
    FlipperGattCharacteristicDataFixed,
    FlipperGattCharacteristicDataCallback,
} FlipperGattCharacteristicDataType;

typedef struct {
    Char_Desc_Uuid_t uuid;
    struct {
        cbFlipperGattCharacteristicData fn;
        const void* context;
    } data_callback;
    uint8_t uuid_type;
    uint8_t max_length;
    uint8_t security_permissions;
    uint8_t access_permissions;
    uint8_t gatt_evt_mask;
    uint8_t is_variable;
} FlipperGattCharacteristicDescriptorParams;

typedef struct {
    const char* name;
    FlipperGattCharacteristicDescriptorParams* descriptor_params;
    union {
        struct {
            const uint8_t* ptr;
            uint16_t length;
        } fixed;
        struct {
            cbFlipperGattCharacteristicData fn;
            const void* context;
        } callback;
    } data;
    Char_UUID_t uuid;
    // Some packed bitfields to save space
    FlipperGattCharacteristicDataType data_prop_type : 2;
    uint8_t is_variable : 2;
    uint8_t uuid_type : 2;
    uint8_t char_properties;
    uint8_t security_permissions;
    uint8_t gatt_evt_mask;
} FlipperGattCharacteristicParams;

_Static_assert(
    sizeof(FlipperGattCharacteristicParams) == 36,
    "FlipperGattCharacteristicParams size must be 36 bytes");

typedef struct {
    const FlipperGattCharacteristicParams* characteristic;
    uint16_t handle;
    uint16_t descriptor_handle;
} FlipperGattCharacteristicInstance;

// Initialize a characteristic instance; copies the characteristic descriptor into the instance
void flipper_gatt_characteristic_init(
    uint16_t svc_handle,
    const FlipperGattCharacteristicParams* char_descriptor,
    FlipperGattCharacteristicInstance* char_instance);

// Delete a characteristic instance; frees the copied characteristic descriptor from the instance
void flipper_gatt_characteristic_delete(
    uint16_t svc_handle,
    FlipperGattCharacteristicInstance* char_instance);

// Update a characteristic instance; if source==NULL, uses the data from the characteristic
//  - For fixed data, fixed.ptr is used as the source if source==NULL
//  - For callback-based data, collback.context is passed as the context if source==NULL
bool flipper_gatt_characteristic_update(
    uint16_t svc_handle,
    FlipperGattCharacteristicInstance* char_instance,
    const void* source);

#ifdef __cplusplus
}
#endif