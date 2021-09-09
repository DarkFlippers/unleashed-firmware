#include "battery_service.h"
#include "app_common.h"
#include "ble.h"

#include <furi.h>

#define BATTERY_SERVICE_TAG "battery service"

typedef struct {
    uint16_t svc_handle;
    uint16_t char_level_handle;
} BatterySvc;

static BatterySvc battery_svc;

bool battery_svc_init() {
    tBleStatus status;
    const uint16_t service_uuid = BATTERY_SERVICE_UUID;
    const uint16_t char_battery_level_uuid = BATTERY_LEVEL_CHAR_UUID;

    // Add Battery service
    status = aci_gatt_add_service(UUID_TYPE_16, (Service_UUID_t*)&service_uuid, PRIMARY_SERVICE, 4, &battery_svc.svc_handle);
    if(status) {
        FURI_LOG_E(BATTERY_SERVICE_TAG, "Failed to add Battery service: %d", status);
    }

    // Add Battery level characteristic
    status = aci_gatt_add_char(battery_svc.svc_handle,
                                UUID_TYPE_16,
                                (Char_UUID_t *) &char_battery_level_uuid,
                                1,
                                CHAR_PROP_READ | CHAR_PROP_NOTIFY,
                                ATTR_PERMISSION_NONE,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_CONSTANT,
                                &battery_svc.char_level_handle);
    if(status) {
        FURI_LOG_E(BATTERY_SERVICE_TAG, "Failed to add Battery level characteristic: %d", status);
    }
    return status != BLE_STATUS_SUCCESS;
}

bool battery_svc_update_level(uint8_t battery_charge) {
    FURI_LOG_I(BATTERY_SERVICE_TAG, "Updating battery level characteristic");
    tBleStatus result = aci_gatt_update_char_value(battery_svc.svc_handle,
                                          battery_svc.char_level_handle,
                                          0,
                                          1,
                                          &battery_charge);
    if(result) {
        FURI_LOG_E(BATTERY_SERVICE_TAG, "Failed updating RX characteristic: %d", result);
    }
    return result != BLE_STATUS_SUCCESS;
}
