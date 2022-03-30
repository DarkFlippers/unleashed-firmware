#include "battery_service.h"
#include "app_common.h"
#include "ble.h"

#include <furi.h>

#define TAG "BtBatterySvc"

typedef struct {
    uint16_t svc_handle;
    uint16_t char_level_handle;
} BatterySvc;

static BatterySvc* battery_svc = NULL;

static const uint16_t service_uuid = BATTERY_SERVICE_UUID;
static const uint16_t char_battery_level_uuid = BATTERY_LEVEL_CHAR_UUID;

void battery_svc_start() {
    battery_svc = malloc(sizeof(BatterySvc));
    tBleStatus status;

    // Add Battery service
    status = aci_gatt_add_service(
        UUID_TYPE_16, (Service_UUID_t*)&service_uuid, PRIMARY_SERVICE, 4, &battery_svc->svc_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add Battery service: %d", status);
    }
    // Add Battery level characteristic
    status = aci_gatt_add_char(
        battery_svc->svc_handle,
        UUID_TYPE_16,
        (Char_UUID_t*)&char_battery_level_uuid,
        1,
        CHAR_PROP_READ | CHAR_PROP_NOTIFY,
        ATTR_PERMISSION_AUTHEN_READ,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &battery_svc->char_level_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add Battery level characteristic: %d", status);
    }
}

void battery_svc_stop() {
    tBleStatus status;
    if(battery_svc) {
        // Delete Battery level characteristic
        status = aci_gatt_del_char(battery_svc->svc_handle, battery_svc->char_level_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Battery level characteristic: %d", status);
        }
        // Delete Battery service
        status = aci_gatt_del_service(battery_svc->svc_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Battery service: %d", status);
        }
        free(battery_svc);
        battery_svc = NULL;
    }
}

bool battery_svc_is_started() {
    return battery_svc != NULL;
}

bool battery_svc_update_level(uint8_t battery_charge) {
    // Check if service was started
    if(battery_svc == NULL) {
        return false;
    }
    // Update battery level characteristic
    FURI_LOG_D(TAG, "Updating battery level characteristic");
    tBleStatus result = aci_gatt_update_char_value(
        battery_svc->svc_handle, battery_svc->char_level_handle, 0, 1, &battery_charge);
    if(result) {
        FURI_LOG_E(TAG, "Failed updating RX characteristic: %d", result);
    }
    return result != BLE_STATUS_SUCCESS;
}
