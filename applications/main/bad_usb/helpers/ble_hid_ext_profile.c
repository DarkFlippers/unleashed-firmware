#include "ble_hid_ext_profile.h"

#include <furi.h>

static FuriHalBleProfileBase* ble_profile_hid_ext_start(FuriHalBleProfileParams profile_params) {
    UNUSED(profile_params);

    return ble_profile_hid->start(NULL);
}

static void ble_profile_hid_ext_stop(FuriHalBleProfileBase* profile) {
    ble_profile_hid->stop(profile);
}

static void
    ble_profile_hid_ext_get_config(GapConfig* config, FuriHalBleProfileParams profile_params) {
    furi_check(config);
    furi_check(profile_params);
    BleProfileHidExtParams* hid_ext_profile_params = profile_params;

    // Setup config with basic profile
    ble_profile_hid->get_gap_config(config, NULL);

    // Set MAC address
    memcpy(config->mac_address, hid_ext_profile_params->mac, sizeof(config->mac_address));

    // Set advertise name (skip first byte which is the ADV type)
    strlcpy(config->adv_name + 1, hid_ext_profile_params->name, sizeof(config->adv_name) - 1);

    // Set bonding mode
    config->bonding_mode = hid_ext_profile_params->bonding;

    // Set pairing method
    config->pairing_method = hid_ext_profile_params->pairing;
}

static const FuriHalBleProfileTemplate profile_callbacks = {
    .start = ble_profile_hid_ext_start,
    .stop = ble_profile_hid_ext_stop,
    .get_gap_config = ble_profile_hid_ext_get_config,
};

const FuriHalBleProfileTemplate* ble_profile_hid_ext = &profile_callbacks;
