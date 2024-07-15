#include "battery_service.h"
#include "app_common.h"
#include <core/check.h>
#include <furi_ble/gatt.h>

#include <ble/ble.h>

#include <furi.h>

#include <m-list.h>

#define TAG "BtBatterySvc"

enum {
    /* Common states */
    BatterySvcPowerStateUnknown = 0b00,
    BatterySvcPowerStateUnsupported = 0b01,
    /* Level states */
    BatterySvcPowerStateGoodLevel = 0b10,
    BatterySvcPowerStateCriticallyLowLevel = 0b11,
    /* Charging states */
    BatterySvcPowerStateNotCharging = 0b10,
    BatterySvcPowerStateCharging = 0b11,
    /* Discharging states */
    BatterySvcPowerStateNotDischarging = 0b10,
    BatterySvcPowerStateDischarging = 0b11,
    /* Battery states */
    BatterySvcPowerStateBatteryNotPresent = 0b10,
    BatterySvcPowerStateBatteryPresent = 0b11,
};

typedef struct {
    uint8_t present     : 2;
    uint8_t discharging : 2;
    uint8_t charging    : 2;
    uint8_t level       : 2;
} BattrySvcPowerState;

_Static_assert(sizeof(BattrySvcPowerState) == 1, "Incorrect structure size");

#define BATTERY_POWER_STATE (0x2A1A)

static const uint16_t service_uuid = BATTERY_SERVICE_UUID;

typedef enum {
    BatterySvcGattCharacteristicBatteryLevel = 0,
    BatterySvcGattCharacteristicPowerState,
    BatterySvcGattCharacteristicCount,
} BatterySvcGattCharacteristicId;

static const BleGattCharacteristicParams battery_svc_chars[BatterySvcGattCharacteristicCount] = {
    [BatterySvcGattCharacteristicBatteryLevel] =
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

struct BleServiceBattery {
    uint16_t svc_handle;
    BleGattCharacteristicInstance chars[BatterySvcGattCharacteristicCount];
    bool auto_update;
};

LIST_DEF(BatterySvcInstanceList, BleServiceBattery*, M_POD_OPLIST);

/* We need to keep track of all battery service instances so that we can update 
 * them when the battery state changes. */
static BatterySvcInstanceList_t instances;
static bool instances_initialized = false;

BleServiceBattery* ble_svc_battery_start(bool auto_update) {
    BleServiceBattery* battery_svc = malloc(sizeof(BleServiceBattery));

    if(!ble_gatt_service_add(
           UUID_TYPE_16,
           (Service_UUID_t*)&service_uuid,
           PRIMARY_SERVICE,
           8,
           &battery_svc->svc_handle)) {
        free(battery_svc);
        return NULL;
    }
    for(size_t i = 0; i < BatterySvcGattCharacteristicCount; i++) {
        ble_gatt_characteristic_init(
            battery_svc->svc_handle, &battery_svc_chars[i], &battery_svc->chars[i]);
    }

    battery_svc->auto_update = auto_update;
    if(auto_update) {
        if(!instances_initialized) {
            BatterySvcInstanceList_init(instances);
            instances_initialized = true;
        }

        BatterySvcInstanceList_push_back(instances, battery_svc);
    }

    return battery_svc;
}

void ble_svc_battery_stop(BleServiceBattery* battery_svc) {
    furi_check(battery_svc);
    if(battery_svc->auto_update) {
        BatterySvcInstanceList_it_t it;
        for(BatterySvcInstanceList_it(it, instances); !BatterySvcInstanceList_end_p(it);
            BatterySvcInstanceList_next(it)) {
            if(*BatterySvcInstanceList_ref(it) == battery_svc) {
                BatterySvcInstanceList_remove(instances, it);
                break;
            }
        }
    }

    for(size_t i = 0; i < BatterySvcGattCharacteristicCount; i++) {
        ble_gatt_characteristic_delete(battery_svc->svc_handle, &battery_svc->chars[i]);
    }
    /* Delete Battery service */
    ble_gatt_service_delete(battery_svc->svc_handle);
    free(battery_svc);
}

bool ble_svc_battery_update_level(BleServiceBattery* battery_svc, uint8_t battery_charge) {
    furi_check(battery_svc);
    /* Update battery level characteristic */
    return ble_gatt_characteristic_update(
        battery_svc->svc_handle,
        &battery_svc->chars[BatterySvcGattCharacteristicBatteryLevel],
        &battery_charge);
}

bool ble_svc_battery_update_power_state(BleServiceBattery* battery_svc, bool charging) {
    furi_check(battery_svc);

    /* Update power state characteristic */
    BattrySvcPowerState power_state = {
        .level = BatterySvcPowerStateUnsupported,
        .present = BatterySvcPowerStateBatteryPresent,
    };
    if(charging) {
        power_state.charging = BatterySvcPowerStateCharging;
        power_state.discharging = BatterySvcPowerStateNotDischarging;
    } else {
        power_state.charging = BatterySvcPowerStateNotCharging;
        power_state.discharging = BatterySvcPowerStateDischarging;
    }

    return ble_gatt_characteristic_update(
        battery_svc->svc_handle,
        &battery_svc->chars[BatterySvcGattCharacteristicPowerState],
        &power_state);
}

void ble_svc_battery_state_update(uint8_t* battery_level, bool* charging) {
    if(!instances_initialized) {
#ifdef FURI_BLE_EXTRA_LOG
        FURI_LOG_W(TAG, "Battery service not initialized");
#endif
        return;
    }

    BatterySvcInstanceList_it_t it;
    for(BatterySvcInstanceList_it(it, instances); !BatterySvcInstanceList_end_p(it);
        BatterySvcInstanceList_next(it)) {
        BleServiceBattery* battery_svc = *BatterySvcInstanceList_ref(it);
        if(battery_level) {
            ble_svc_battery_update_level(battery_svc, *battery_level);
        }
        if(charging) {
            ble_svc_battery_update_power_state(battery_svc, *charging);
        }
    }
}
