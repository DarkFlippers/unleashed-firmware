#include "battery_service.h"
#include "app_common.h"
#include "gatt_char.h"

#include <ble/ble.h>

#include <furi.h>
#include <furi_hal_power.h>

#define TAG "BtBatterySvc"

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

#define BATTERY_POWER_STATE (0x2A1A)

static const uint16_t service_uuid = BATTERY_SERVICE_UUID;

typedef enum {
    BatterySvcGattCharacteristicBatteryLevel = 0,
    BatterySvcGattCharacteristicPowerState,
    BatterySvcGattCharacteristicCount,
} BatterySvcGattCharacteristicId;

static const FlipperGattCharacteristicParams battery_svc_chars[BatterySvcGattCharacteristicCount] =
    {[BatterySvcGattCharacteristicBatteryLevel] =
         {.name = "Battery Level",
          .data_prop_type = FlipperGattCharacteristicDataFixed,
          .data.fixed.length = 1,
          .uuid.Char_UUID_16 = BATTERY_LEVEL_CHAR_UUID,
          .uuid_type = UUID_TYPE_16,
          .char_properties = CHAR_PROP_READ | CHAR_PROP_NOTIFY,
          .security_permissions = ATTR_PERMISSION_AUTHEN_READ,
          .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
          .is_variable = CHAR_VALUE_LEN_CONSTANT},
     [BatterySvcGattCharacteristicPowerState] = {
         .name = "Power State",
         .data_prop_type = FlipperGattCharacteristicDataFixed,
         .data.fixed.length = 1,
         .uuid.Char_UUID_16 = BATTERY_POWER_STATE,
         .uuid_type = UUID_TYPE_16,
         .char_properties = CHAR_PROP_READ | CHAR_PROP_NOTIFY,
         .security_permissions = ATTR_PERMISSION_AUTHEN_READ,
         .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
         .is_variable = CHAR_VALUE_LEN_CONSTANT}};

typedef struct {
    uint16_t svc_handle;
    FlipperGattCharacteristicInstance chars[BatterySvcGattCharacteristicCount];
} BatterySvc;

static BatterySvc* battery_svc = NULL;

void battery_svc_start() {
    battery_svc = malloc(sizeof(BatterySvc));
    tBleStatus status;

    // Add Battery service
    status = aci_gatt_add_service(
        UUID_TYPE_16, (Service_UUID_t*)&service_uuid, PRIMARY_SERVICE, 8, &battery_svc->svc_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add Battery service: %d", status);
    }
    for(size_t i = 0; i < BatterySvcGattCharacteristicCount; i++) {
        flipper_gatt_characteristic_init(
            battery_svc->svc_handle, &battery_svc_chars[i], &battery_svc->chars[i]);
    }

    battery_svc_update_power_state();
}

void battery_svc_stop() {
    tBleStatus status;
    if(battery_svc) {
        for(size_t i = 0; i < BatterySvcGattCharacteristicCount; i++) {
            flipper_gatt_characteristic_delete(battery_svc->svc_handle, &battery_svc->chars[i]);
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
    return flipper_gatt_characteristic_update(
        battery_svc->svc_handle,
        &battery_svc->chars[BatterySvcGattCharacteristicBatteryLevel],
        &battery_charge);
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

    return flipper_gatt_characteristic_update(
        battery_svc->svc_handle,
        &battery_svc->chars[BatterySvcGattCharacteristicPowerState],
        &power_state);
}
