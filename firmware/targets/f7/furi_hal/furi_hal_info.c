#include <furi_hal_info.h>
#include <furi_hal_region.h>
#include <furi_hal_version.h>
#include <furi_hal_bt.h>
#include <furi_hal_crypto.h>
#include <furi_hal_rtc.h>

#include <interface/patterns/ble_thread/shci/shci.h>
#include <furi.h>
#include <protobuf_version.h>

FURI_WEAK void furi_hal_info_get_api_version(uint16_t* major, uint16_t* minor) {
    *major = 0;
    *minor = 0;
}

void furi_hal_info_get(PropertyValueCallback out, char sep, void* context) {
    FuriString* key = furi_string_alloc();
    FuriString* value = furi_string_alloc();

    PropertyValueContext property_context = {
        .key = key, .value = value, .out = out, .sep = sep, .last = false, .context = context};

    // Device Info version
    if(sep == '.') {
        property_value_out(&property_context, NULL, 2, "format", "major", "3");
        property_value_out(&property_context, NULL, 2, "format", "minor", "3");
    } else {
        property_value_out(&property_context, NULL, 3, "device", "info", "major", "2");
        property_value_out(&property_context, NULL, 3, "device", "info", "minor", "4");
    }

    // Model name
    property_value_out(
        &property_context, NULL, 2, "hardware", "model", furi_hal_version_get_model_name());

    // Unique ID
    furi_string_reset(value);
    const uint8_t* uid = furi_hal_version_uid();
    for(size_t i = 0; i < furi_hal_version_uid_size(); i++) {
        furi_string_cat_printf(value, "%02X", uid[i]);
    }
    property_value_out(&property_context, NULL, 2, "hardware", "uid", furi_string_get_cstr(value));

    // OTP Revision
    property_value_out(
        &property_context, "%d", 3, "hardware", "otp", "ver", furi_hal_version_get_otp_version());
    property_value_out(
        &property_context, "%lu", 2, "hardware", "timestamp", furi_hal_version_get_hw_timestamp());

    // Board Revision
    property_value_out(
        &property_context, "%d", 2, "hardware", "ver", furi_hal_version_get_hw_version());
    property_value_out(
        &property_context, "%d", 2, "hardware", "target", furi_hal_version_get_hw_target());
    property_value_out(
        &property_context, "%d", 2, "hardware", "body", furi_hal_version_get_hw_body());
    property_value_out(
        &property_context, "%d", 2, "hardware", "connect", furi_hal_version_get_hw_connect());
    property_value_out(
        &property_context, "%d", 2, "hardware", "display", furi_hal_version_get_hw_display());

    // Board Personification
    property_value_out(
        &property_context, "%d", 2, "hardware", "color", furi_hal_version_get_hw_color());

    if(sep == '.') {
        property_value_out(
            &property_context,
            "%d",
            3,
            "hardware",
            "region",
            "builtin",
            furi_hal_version_get_hw_region());
    } else {
        property_value_out(
            &property_context, "%d", 2, "hardware", "region", furi_hal_version_get_hw_region());
    }

    property_value_out(
        &property_context,
        NULL,
        3,
        "hardware",
        "region",
        "provisioned",
        furi_hal_region_get_name());

    const char* name = furi_hal_version_get_name_ptr();
    if(name) {
        property_value_out(&property_context, NULL, 2, "hardware", "name", name);
    }

    // Firmware version
    const Version* firmware_version = furi_hal_version_get_firmware_version();
    if(firmware_version) {
        if(sep == '.') {
            property_value_out(
                &property_context,
                NULL,
                3,
                "firmware",
                "commit",
                "hash",
                version_get_githash(firmware_version));
        } else {
            property_value_out(
                &property_context,
                NULL,
                2,
                "firmware",
                "commit",
                version_get_githash(firmware_version));
        }

        property_value_out(
            &property_context,
            NULL,
            3,
            "firmware",
            "commit",
            "dirty",
            version_get_dirty_flag(firmware_version) ? "true" : "false");

        if(sep == '.') {
            property_value_out(
                &property_context,
                NULL,
                3,
                "firmware",
                "branch",
                "name",
                version_get_gitbranch(firmware_version));
        } else {
            property_value_out(
                &property_context,
                NULL,
                2,
                "firmware",
                "branch",
                version_get_gitbranch(firmware_version));
        }

        property_value_out(
            &property_context,
            NULL,
            3,
            "firmware",
            "branch",
            "num",
            version_get_gitbranchnum(firmware_version));
        property_value_out(
            &property_context,
            NULL,
            2,
            "firmware",
            "version",
            version_get_version(firmware_version));
        property_value_out(
            &property_context,
            NULL,
            3,
            "firmware",
            "build",
            "date",
            version_get_builddate(firmware_version));
        property_value_out(
            &property_context, "%d", 2, "firmware", "target", version_get_target(firmware_version));

        uint16_t api_version_major, api_version_minor;
        furi_hal_info_get_api_version(&api_version_major, &api_version_minor);
        property_value_out(
            &property_context, "%d", 3, "firmware", "api", "major", api_version_major);
        property_value_out(
            &property_context, "%d", 3, "firmware", "api", "minor", api_version_minor);

        property_value_out(
            &property_context,
            NULL,
            3,
            "firmware",
            "origin",
            "fork",
            version_get_firmware_origin(firmware_version));

        property_value_out(
            &property_context,
            NULL,
            3,
            "firmware",
            "origin",
            "git",
            version_get_git_origin(firmware_version));
    }

    if(furi_hal_bt_is_alive()) {
        const BleGlueC2Info* ble_c2_info = ble_glue_get_c2_info();
        property_value_out(&property_context, NULL, 2, "radio", "alive", "true");
        property_value_out(
            &property_context,
            NULL,
            2,
            "radio",
            "mode",
            ble_c2_info->mode == BleGlueC2ModeFUS ? "FUS" : "Stack");

        // FUS Info
        property_value_out(
            &property_context, "%d", 3, "radio", "fus", "major", ble_c2_info->FusVersionMajor);
        property_value_out(
            &property_context, "%d", 3, "radio", "fus", "minor", ble_c2_info->FusVersionMinor);
        property_value_out(
            &property_context, "%d", 3, "radio", "fus", "sub", ble_c2_info->FusVersionSub);
        property_value_out(
            &property_context,
            "%dK",
            3,
            "radio",
            "fus",
            "sram2b",
            ble_c2_info->FusMemorySizeSram2B);
        property_value_out(
            &property_context,
            "%dK",
            3,
            "radio",
            "fus",
            "sram2a",
            ble_c2_info->FusMemorySizeSram2A);
        property_value_out(
            &property_context,
            "%dK",
            3,
            "radio",
            "fus",
            "flash",
            ble_c2_info->FusMemorySizeFlash * 4);

        // Stack Info
        property_value_out(
            &property_context, "%d", 3, "radio", "stack", "type", ble_c2_info->StackType);
        property_value_out(
            &property_context, "%d", 3, "radio", "stack", "major", ble_c2_info->VersionMajor);
        property_value_out(
            &property_context, "%d", 3, "radio", "stack", "minor", ble_c2_info->VersionMinor);
        property_value_out(
            &property_context, "%d", 3, "radio", "stack", "sub", ble_c2_info->VersionSub);
        property_value_out(
            &property_context, "%d", 3, "radio", "stack", "branch", ble_c2_info->VersionBranch);
        property_value_out(
            &property_context,
            "%d",
            3,
            "radio",
            "stack",
            "release",
            ble_c2_info->VersionReleaseType);
        property_value_out(
            &property_context, "%dK", 3, "radio", "stack", "sram2b", ble_c2_info->MemorySizeSram2B);
        property_value_out(
            &property_context, "%dK", 3, "radio", "stack", "sram2a", ble_c2_info->MemorySizeSram2A);
        property_value_out(
            &property_context, "%dK", 3, "radio", "stack", "sram1", ble_c2_info->MemorySizeSram1);
        property_value_out(
            &property_context,
            "%dK",
            3,
            "radio",
            "stack",
            "flash",
            ble_c2_info->MemorySizeFlash * 4);

        // Mac address
        furi_string_reset(value);
        const uint8_t* ble_mac = furi_hal_version_get_ble_mac();
        for(size_t i = 0; i < 6; i++) {
            furi_string_cat_printf(value, "%02X", ble_mac[i]);
        }
        property_value_out(
            &property_context, NULL, 3, "radio", "ble", "mac", furi_string_get_cstr(value));

        // Signature verification
        uint8_t enclave_keys = 0;
        uint8_t enclave_valid_keys = 0;
        bool enclave_valid = furi_hal_crypto_enclave_verify(&enclave_keys, &enclave_valid_keys);
        if(sep == '.') {
            property_value_out(
                &property_context, "%d", 3, "enclave", "keys", "valid", enclave_valid_keys);
        } else {
            property_value_out(
                &property_context, "%d", 3, "enclave", "valid", "keys", enclave_valid_keys);
        }

        property_value_out(
            &property_context, NULL, 2, "enclave", "valid", enclave_valid ? "true" : "false");
    } else {
        property_value_out(&property_context, NULL, 2, "radio", "alive", "false");
    }

    // RTC flags
    property_value_out(
        &property_context,
        "%u",
        2,
        "system",
        "debug",
        furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug));
    property_value_out(
        &property_context, "%u", 2, "system", "lock", furi_hal_rtc_is_flag_set(FuriHalRtcFlagLock));
    property_value_out(
        &property_context,
        "%u",
        2,
        "system",
        "orient",
        furi_hal_rtc_is_flag_set(FuriHalRtcFlagHandOrient));
    property_value_out(
        &property_context,
        "%u",
        3,
        "system",
        "sleep",
        "legacy",
        furi_hal_rtc_is_flag_set(FuriHalRtcFlagLegacySleep));
    property_value_out(
        &property_context,
        "%u",
        2,
        "system",
        "stealth",
        furi_hal_rtc_is_flag_set(FuriHalRtcFlagStealthMode));

    property_value_out(
        &property_context, "%u", 3, "system", "heap", "track", furi_hal_rtc_get_heap_track_mode());
    property_value_out(&property_context, "%u", 2, "system", "boot", furi_hal_rtc_get_boot_mode());
    property_value_out(
        &property_context,
        "%u",
        3,
        "system",
        "locale",
        "time",
        furi_hal_rtc_get_locale_timeformat());
    property_value_out(
        &property_context,
        "%u",
        3,
        "system",
        "locale",
        "date",
        furi_hal_rtc_get_locale_dateformat());
    property_value_out(
        &property_context, "%u", 3, "system", "locale", "unit", furi_hal_rtc_get_locale_units());
    property_value_out(
        &property_context, "%u", 3, "system", "log", "level", furi_hal_rtc_get_log_level());

    property_value_out(
        &property_context, "%u", 3, "protobuf", "version", "major", PROTOBUF_MAJOR_VERSION);
    property_context.last = true;
    property_value_out(
        &property_context, "%u", 3, "protobuf", "version", "minor", PROTOBUF_MINOR_VERSION);

    furi_string_free(key);
    furi_string_free(value);
}
