#include "serial_profile.h"

#include <gap.h>
#include <furi_ble/profile_interface.h>
#include <services/dev_info_service.h>
#include <services/battery_service.h>
#include <services/serial_service.h>
#include <furi.h>

typedef struct {
    FuriHalBleProfileBase base;

    BleServiceDevInfo* dev_info_svc;
    BleServiceBattery* battery_svc;
    BleServiceSerial* serial_svc;
} BleProfileSerial;
_Static_assert(offsetof(BleProfileSerial, base) == 0, "Wrong layout");

static FuriHalBleProfileBase* ble_profile_serial_start(FuriHalBleProfileParams profile_params) {
    UNUSED(profile_params);

    BleProfileSerial* profile = malloc(sizeof(BleProfileSerial));

    profile->base.config = ble_profile_serial;

    profile->dev_info_svc = ble_svc_dev_info_start();
    profile->battery_svc = ble_svc_battery_start(true);
    profile->serial_svc = ble_svc_serial_start();

    return &profile->base;
}

static void ble_profile_serial_stop(FuriHalBleProfileBase* profile) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_serial);

    BleProfileSerial* serial_profile = (BleProfileSerial*)profile;
    ble_svc_battery_stop(serial_profile->battery_svc);
    ble_svc_dev_info_stop(serial_profile->dev_info_svc);
    ble_svc_serial_stop(serial_profile->serial_svc);
}

// AN5289: 4.7, in order to use flash controller interval must be at least 25ms + advertisement, which is 30 ms
// Since we don't use flash controller anymore interval can be lowered to 7.5ms
#define CONNECTION_INTERVAL_MIN (0x06)
// Up to 45 ms
#define CONNECTION_INTERVAL_MAX (0x24)

static GapConfig serial_template_config = {
    .adv_service_uuid = 0x3080,
    .appearance_char = 0x8600,
    .bonding_mode = true,
    .pairing_method = GapPairingPinCodeShow,
    .conn_param = {
        .conn_int_min = CONNECTION_INTERVAL_MIN,
        .conn_int_max = CONNECTION_INTERVAL_MAX,
        .slave_latency = 0,
        .supervisor_timeout = 0,
    }};

static void
    ble_profile_serial_get_config(GapConfig* config, FuriHalBleProfileParams profile_params) {
    UNUSED(profile_params);

    furi_check(config);
    memcpy(config, &serial_template_config, sizeof(GapConfig));
    // Set mac address
    memcpy(config->mac_address, furi_hal_version_get_ble_mac(), sizeof(config->mac_address));
    // Set advertise name
    strlcpy(
        config->adv_name,
        furi_hal_version_get_ble_local_device_name_ptr(),
        FURI_HAL_VERSION_DEVICE_NAME_LENGTH);
    config->adv_service_uuid |= furi_hal_version_get_hw_color();
}

static const FuriHalBleProfileTemplate profile_callbacks = {
    .start = ble_profile_serial_start,
    .stop = ble_profile_serial_stop,
    .get_gap_config = ble_profile_serial_get_config,
};

const FuriHalBleProfileTemplate* ble_profile_serial = &profile_callbacks;

void ble_profile_serial_set_event_callback(
    FuriHalBleProfileBase* profile,
    uint16_t buff_size,
    FuriHalBtSerialCallback callback,
    void* context) {
    furi_check(profile && (profile->config == ble_profile_serial));

    BleProfileSerial* serial_profile = (BleProfileSerial*)profile;
    ble_svc_serial_set_callbacks(serial_profile->serial_svc, buff_size, callback, context);
}

void ble_profile_serial_notify_buffer_is_empty(FuriHalBleProfileBase* profile) {
    furi_check(profile && (profile->config == ble_profile_serial));

    BleProfileSerial* serial_profile = (BleProfileSerial*)profile;
    ble_svc_serial_notify_buffer_is_empty(serial_profile->serial_svc);
}

void ble_profile_serial_set_rpc_active(FuriHalBleProfileBase* profile, bool active) {
    furi_check(profile && (profile->config == ble_profile_serial));

    BleProfileSerial* serial_profile = (BleProfileSerial*)profile;
    ble_svc_serial_set_rpc_active(serial_profile->serial_svc, active);
}

bool ble_profile_serial_tx(FuriHalBleProfileBase* profile, uint8_t* data, uint16_t size) {
    furi_check(profile && (profile->config == ble_profile_serial));

    BleProfileSerial* serial_profile = (BleProfileSerial*)profile;

    if(size > BLE_PROFILE_SERIAL_PACKET_SIZE_MAX) {
        return false;
    }

    return ble_svc_serial_update_tx(serial_profile->serial_svc, data, size);
}
