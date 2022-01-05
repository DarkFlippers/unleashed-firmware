#include <furi_hal_info.h>
#include <furi_hal.h>
#include <shci.h>

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
    const char* name = furi_hal_version_get_name_ptr();
    if(name) {
        out("hardware_name", name, false, context);
    }

    // Bootloader Version
    const Version* bootloader_version = furi_hal_version_get_bootloader_version();
    if(bootloader_version) {
        out("bootloader_commit", version_get_githash(bootloader_version), false, context);
        out("bootloader_branch", version_get_gitbranch(bootloader_version), false, context);
        out("bootloader_branch_num", version_get_gitbranchnum(bootloader_version), false, context);
        out("bootloader_version", version_get_version(bootloader_version), false, context);
        out("bootloader_build_date", version_get_builddate(bootloader_version), false, context);
        string_printf(value, "%d", version_get_target(bootloader_version));
        out("bootloader_target", string_get_cstr(value), false, context);
    }

    // Firmware version
    const Version* firmware_version = furi_hal_version_get_firmware_version();
    if(firmware_version) {
        out("firmware_commit", version_get_githash(firmware_version), false, context);
        out("firmware_branch", version_get_gitbranch(firmware_version), false, context);
        out("firmware_branch_num", version_get_gitbranchnum(firmware_version), false, context);
        out("firmware_version", version_get_version(firmware_version), false, context);
        out("firmware_build_date", version_get_builddate(firmware_version), false, context);
        string_printf(value, "%d", version_get_target(firmware_version));
        out("firmware_target", string_get_cstr(value), false, context);
    }

    WirelessFwInfo_t pWirelessInfo;
    if(furi_hal_bt_is_alive() && SHCI_GetWirelessFwInfo(&pWirelessInfo) == SHCI_Success) {
        out("radio_alive", "true", false, context);

        // FUS Info
        string_printf(value, "%d", pWirelessInfo.FusVersionMajor);
        out("radio_fus_major", string_get_cstr(value), false, context);
        string_printf(value, "%d", pWirelessInfo.FusVersionMinor);
        out("radio_fus_minor", string_get_cstr(value), false, context);
        string_printf(value, "%d", pWirelessInfo.FusVersionSub);
        out("radio_fus_sub", string_get_cstr(value), false, context);
        string_printf(value, "%dK", pWirelessInfo.FusMemorySizeSram2B);
        out("radio_fus_sram2b", string_get_cstr(value), false, context);
        string_printf(value, "%dK", pWirelessInfo.FusMemorySizeSram2A);
        out("radio_fus_sram2a", string_get_cstr(value), false, context);
        string_printf(value, "%dK", pWirelessInfo.FusMemorySizeFlash * 4);
        out("radio_fus_flash", string_get_cstr(value), false, context);

        // Stack Info
        string_printf(value, "%d", pWirelessInfo.StackType);
        out("radio_stack_type", string_get_cstr(value), false, context);
        string_printf(value, "%d", pWirelessInfo.VersionMajor);
        out("radio_stack_major", string_get_cstr(value), false, context);
        string_printf(value, "%d", pWirelessInfo.VersionMinor);
        out("radio_stack_minor", string_get_cstr(value), false, context);
        string_printf(value, "%d", pWirelessInfo.VersionSub);
        out("radio_stack_sub", string_get_cstr(value), false, context);
        string_printf(value, "%d", pWirelessInfo.VersionBranch);
        out("radio_stack_branch", string_get_cstr(value), false, context);
        string_printf(value, "%d", pWirelessInfo.VersionReleaseType);
        out("radio_stack_release", string_get_cstr(value), false, context);
        string_printf(value, "%dK", pWirelessInfo.MemorySizeSram2B);
        out("radio_stack_sram2b", string_get_cstr(value), false, context);
        string_printf(value, "%dK", pWirelessInfo.MemorySizeSram2A);
        out("radio_stack_sram2a", string_get_cstr(value), false, context);
        string_printf(value, "%dK", pWirelessInfo.MemorySizeSram1);
        out("radio_stack_sram1", string_get_cstr(value), false, context);
        string_printf(value, "%dK", pWirelessInfo.MemorySizeFlash * 4);
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
        out("enclave_valid", enclave_valid ? "true" : "false", true, context);
    } else {
        out("radio_alive", "false", true, context);
    }

    string_clear(value);
}