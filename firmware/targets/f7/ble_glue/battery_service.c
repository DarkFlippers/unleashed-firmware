#include "battery_service.h"
#include "app_common.h"
#include <ble/ble.h>

#include <furi.h>
#include <furi_hal_power.h>

#define TAG "BtBatterySvc"

typedef struct {
    uint16_t svc_handle;
    uint16_t battery_level_char_handle;
    uint16_t power_state_char_handle;
} BatterySvc;

enum {
    // Common states
    BatterySvcPowerStateUnknown = 0b00,
    BatterySvcPowerStateUnsupported = 0b01,
    // Level states
    BatterySvcPowerStateGoodLevel = 0b10,
    BatterySvcPowerStateCriticallyLowLevel = 0b11,
    // Charging states
    BatterySvcPowerStateNotCharging = 0b10,
    BatterySvcPowerStateCharging = 0b11,
    // Discharging states
    BatterySvcPowerStateNotDischarging = 0b10,
    BatterySvcPowerStateDischarging = 0b11,
    // Battery states
    BatterySvcPowerStateBatteryNotPresent = 0b10,
    BatterySvcPowerStateBatteryPresent = 0b11,
};

typedef struct {
    uint8_t present : 2;
    uint8_t discharging : 2;
    uint8_t charging : 2;
    uint8_t level : 2;
} BattrySvcPowerState;

_Static_assert(sizeof(BattrySvcPowerState) == 1, "Incorrect structure size");

static BatterySvc* battery_svc = NULL;

#define BATTERY_POWER_STATE (0x2A1A)

static const uint16_t service_uuid = BATTERY_SERVICE_UUID;
static const uint16_t battery_level_char_uuid = BATTERY_LEVEL_CHAR_UUID;
static const uint16_t power_state_char_uuid = BATTERY_POWER_STATE;

void battery_svc_start() {
    battery_svc = malloc(sizeof(BatterySvc));
    tBleStatus status;

    // Add Battery service
    status = aci_gatt_add_service(
        UUID_TYPE_16, (Service_UUID_t*)&service_uuid, PRIMARY_SERVICE, 8, &battery_svc->svc_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add Battery service: %d", status);
    }
    // Add Battery level characteristic
    status = aci_gatt_add_char(
        battery_svc->svc_handle,
        UUID_TYPE_16,
        (Char_UUID_t*)&battery_level_char_uuid,
        1,
        CHAR_PROP_READ | CHAR_PROP_NOTIFY,
        ATTR_PERMISSION_AUTHEN_READ,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &battery_svc->battery_level_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add Battery level characteristic: %d", status);
    }
    // Add Power state characteristic
    status = aci_gatt_add_char(
        battery_svc->svc_handle,
        UUID_TYPE_16,
        (Char_UUID_t*)&power_state_char_uuid,
        1,
        CHAR_PROP_READ | CHAR_PROP_NOTIFY,
        ATTR_PERMISSION_AUTHEN_READ,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &battery_svc->power_state_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add Battery level characteristic: %d", status);
    }
    // Update power state charachteristic
    battery_svc_update_power_state();
}

void battery_svc_stop() {
    tBleStatus status;
    if(battery_svc) {
        // Delete Battery level characteristic
        status =
            aci_gatt_del_char(battery_svc->svc_handle, battery_svc->battery_level_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete Battery level characteristic: %d", status);
        }
        // Delete Power state characteristic
        status = aci_gatt_del_char(battery_svc->svc_handle, battery_svc->power_state_char_handle);
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
        battery_svc->svc_handle, battery_svc->battery_level_char_handle, 0, 1, &battery_charge);
    if(result) {
        FURI_LOG_E(TAG, "Failed updating RX characteristic: %d", result);
    }
    return result != BLE_STATUS_SUCCESS;
}

bool battery_svc_update_power_state() {
    // Check if service was started
    if(battery_svc == NULL) {
        return false;
    }
    // Update power state characteristic
    BattrySvcPowerState power_state = {
        .level = BatterySvcPowerStateUnsupported,
        .present = BatterySvcPowerStateBatteryPresent,
    };
    if(furi_hal_power_is_charging()) {
        power_state.charging = BatterySvcPowerStateCharging;
        power_state.discharging = BatterySvcPowerStateNotDischarging;
    } else {
        power_state.charging = BatterySvcPowerStateNotCharging;
        power_state.discharging = BatterySvcPowerStateDischarging;
    }
    FURI_LOG_D(TAG, "Updating power state characteristic");
    tBleStatus result = aci_gatt_update_char_value(
        battery_svc->svc_handle,
        battery_svc->power_state_char_handle,
        0,
        1,
        (uint8_t*)&power_state);
    if(result) {
        FURI_LOG_E(TAG, "Failed updating Power state characteristic: %d", result);
    }
    return result != BLE_STATUS_SUCCESS;
}
