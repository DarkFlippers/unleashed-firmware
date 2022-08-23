#include <furi_hal_info.h>
#include <furi_hal_region.h>
#include <furi_hal_version.h>
#include <furi_hal_bt.h>
#include <furi_hal_crypto.h>

#include <interface/patterns/ble_thread/shci/shci.h>
#include <m-string.h>
#include <protobuf_version.h>

void furi_hal_info_get(FuriHalInfoValueCallback out, void* context) {
    string_t value;
    string_init(value);

    // Device Info version
    out("device_info_major", "2", false, context);
    out("device_info_minor", "0", false, context);

    // Model name
    out("hardware_model", furi_hal_version_get_model_name(), false, context);

    // Unique ID
    string_reset(value);
    const uint8_t* uid = furi_hal_version_uid();
    for(size_t i = 0; i < furi_hal_version_uid_size(); i++) {
        string_cat_printf(value, "%02X", uid[i]);
    }
    out("hardware_uid", string_get_cstr(value), false, context);

    // OTP Revision
    string_printf(value, "%d", furi_hal_version_get_otp_version());
    out("hardware_otp_ver", string_get_cstr(value), false, context);
    string_printf(value, "%lu", furi_hal_version_get_hw_timestamp());
    out("hardware_timestamp", string_get_cstr(value), false, context);

    // Board Revision
    string_printf(value, "%d", furi_hal_version_get_hw_version());
    out("hardware_ver", string_get_cstr(value), false, context);
    string_printf(value, "%d", furi_hal_version_get_hw_target());
    out("hardware_target", string_get_cstr(value), false, context);
    string_printf(value, "%d", furi_hal_version_get_hw_body());
    out("hardware_body", string_get_cstr(value), false, context);
    string_printf(value, "%d", furi_hal_version_get_hw_connect());
    out("hardware_connect", string_get_cstr(value), false, context);
    string_printf(value, "%d", furi_hal_version_get_hw_display());
    out("hardware_display", string_get_cstr(value), false, context);

    // Board Personification
    string_printf(value, "%d", furi_hal_version_get_hw_color());
    out("hardware_color", string_get_cstr(value), false, context);
    string_printf(value, "%d", furi_hal_version_get_hw_region());
    out("hardware_region", string_get_cstr(value), false, context);
    out("hardware_region_provisioned", furi_hal_region_get_name(), false, context);
    const char* name = furi_hal_version_get_name_ptr();
    if(name) {
        out("hardware_name", name, false, context);
    }

    // Firmware version
    const Version* firmware_version = furi_hal_version_get_firmware_version();
    if(firmware_version) {
        out("firmware_commit", version_get_githash(firmware_version), false, context);
        out("firmware_commit_dirty",
            version_get_dirty_flag(firmware_version) ? "true" : "false",
            false,
            context);
        out("firmware_branch", version_get_gitbranch(firmware_version), false, context);
        out("firmware_branch_num", version_get_gitbranchnum(firmware_version), false, context);
        out("firmware_version", version_get_version(firmware_version), false, context);
        out("firmware_build_date", version_get_builddate(firmware_version), false, context);
        string_printf(value, "%d", version_get_target(firmware_version));
        out("firmware_target", string_get_cstr(value), false, context);
    }

    if(furi_hal_bt_is_alive()) {
        const BleGlueC2Info* ble_c2_info = ble_glue_get_c2_info();
        out("radio_alive", "true", false, context);
        out("radio_mode", ble_c2_info->mode == BleGlueC2ModeFUS ? "FUS" : "Stack", false, context);

        // FUS Info
        string_printf(value, "%d", ble_c2_info->FusVersionMajor);
        out("radio_fus_major", string_get_cstr(value), false, context);
        string_printf(value, "%d", ble_c2_info->FusVersionMinor);
        out("radio_fus_minor", string_get_cstr(value), false, context);
        string_printf(value, "%d", ble_c2_info->FusVersionSub);
        out("radio_fus_sub", string_get_cstr(value), false, context);
        string_printf(value, "%dK", ble_c2_info->FusMemorySizeSram2B);
        out("radio_fus_sram2b", string_get_cstr(value), false, context);
        string_printf(value, "%dK", ble_c2_info->FusMemorySizeSram2A);
        out("radio_fus_sram2a", string_get_cstr(value), false, context);
        string_printf(value, "%dK", ble_c2_info->FusMemorySizeFlash * 4);
        out("radio_fus_flash", string_get_cstr(value), false, context);

        // Stack Info
        string_printf(value, "%d", ble_c2_info->StackType);
        out("radio_stack_type", string_get_cstr(value), false, context);
        string_printf(value, "%d", ble_c2_info->VersionMajor);
        out("radio_stack_major", string_get_cstr(value), false, context);
        string_printf(value, "%d", ble_c2_info->VersionMinor);
        out("radio_stack_minor", string_get_cstr(value), false, context);
        string_printf(value, "%d", ble_c2_info->VersionSub);
        out("radio_stack_sub", string_get_cstr(value), false, context);
        string_printf(value, "%d", ble_c2_info->VersionBranch);
        out("radio_stack_branch", string_get_cstr(value), false, context);
        string_printf(value, "%d", ble_c2_info->VersionReleaseType);
        out("radio_stack_release", string_get_cstr(value), false, context);
        string_printf(value, "%dK", ble_c2_info->MemorySizeSram2B);
        out("radio_stack_sram2b", string_get_cstr(value), false, context);
        string_printf(value, "%dK", ble_c2_info->MemorySizeSram2A);
        out("radio_stack_sram2a", string_get_cstr(value), false, context);
        string_printf(value, "%dK", ble_c2_info->MemorySizeSram1);
        out("radio_stack_sram1", string_get_cstr(value), false, context);
        string_printf(value, "%dK", ble_c2_info->MemorySizeFlash * 4);
        out("radio_stack_flash", string_get_cstr(value), false, context);

        // Mac address
        string_reset(value);
        const uint8_t* ble_mac = furi_hal_version_get_ble_mac();
        for(size_t i = 0; i < 6; i++) {
            string_cat_printf(value, "%02X", ble_mac[i]);
        }
        out("radio_ble_mac", string_get_cstr(value), false, context);

        // Signature verification
        uint8_t enclave_keys = 0;
        uint8_t enclave_valid_keys = 0;
        bool enclave_valid = furi_hal_crypto_verify_enclave(&enclave_keys, &enclave_valid_keys);
        string_printf(value, "%d", enclave_valid_keys);
        out("enclave_valid_keys", string_get_cstr(value), false, context);
        out("enclave_valid", enclave_valid ? "true" : "false", false, context);
    } else {
        out("radio_alive", "false", false, context);
    }

    string_printf(value, "%u", PROTOBUF_MAJOR_VERSION);
    out("protobuf_version_major", string_get_cstr(value), false, context);
    string_printf(value, "%u", PROTOBUF_MINOR_VERSION);
    out("protobuf_version_minor", string_get_cstr(value), true, context);

    string_clear(value);
}
