#include <furi-hal-info.h>
#include <furi-hal.h>
#include <shci.h>

#define ENCLAVE_SIGNATURE_KEY_SLOTS 10
#define ENCLAVE_SIGNATURE_SIZE 16

static const uint8_t enclave_signature_iv[ENCLAVE_SIGNATURE_KEY_SLOTS][16] = {
    {0xac, 0x5d, 0x68, 0xb8, 0x79, 0x74, 0xfc, 0x7f, 0x45, 0x02, 0x82, 0xf1, 0x48, 0x7e, 0x75, 0x8a},
    {0x38, 0xe6, 0x6a, 0x90, 0x5e, 0x5b, 0x8a, 0xa6, 0x70, 0x30, 0x04, 0x72, 0xc2, 0x42, 0xea, 0xaf},
    {0x73, 0xd5, 0x8e, 0xfb, 0x0f, 0x4b, 0xa9, 0x79, 0x0f, 0xde, 0x0e, 0x53, 0x44, 0x7d, 0xaa, 0xfd},
    {0x3c, 0x9a, 0xf4, 0x43, 0x2b, 0xfe, 0xea, 0xae, 0x8c, 0xc6, 0xd1, 0x60, 0xd2, 0x96, 0x64, 0xa9},
    {0x10, 0xac, 0x7b, 0x63, 0x03, 0x7f, 0x43, 0x18, 0xec, 0x9d, 0x9c, 0xc4, 0x01, 0xdc, 0x35, 0xa7},
    {0x26, 0x21, 0x64, 0xe6, 0xd0, 0xf2, 0x47, 0x49, 0xdc, 0x36, 0xcd, 0x68, 0x0c, 0x91, 0x03, 0x44},
    {0x7a, 0xbd, 0xce, 0x9c, 0x24, 0x7a, 0x2a, 0xb1, 0x3c, 0x4f, 0x5a, 0x7d, 0x80, 0x3e, 0xfc, 0x0d},
    {0xcd, 0xdd, 0xd3, 0x02, 0x85, 0x65, 0x43, 0x83, 0xf9, 0xac, 0x75, 0x2f, 0x21, 0xef, 0x28, 0x6b},
    {0xab, 0x73, 0x70, 0xe8, 0xe2, 0x56, 0x0f, 0x58, 0xab, 0x29, 0xa5, 0xb1, 0x13, 0x47, 0x5e, 0xe8},
    {0x4f, 0x3c, 0x43, 0x77, 0xde, 0xed, 0x79, 0xa1, 0x8d, 0x4c, 0x1f, 0xfd, 0xdb, 0x96, 0x87, 0x2e},
};

static const uint8_t enclave_signature_input[ENCLAVE_SIGNATURE_KEY_SLOTS][ENCLAVE_SIGNATURE_SIZE] = {
    {0x9f, 0x5c, 0xb1, 0x43, 0x17, 0x53, 0x18, 0x8c, 0x66, 0x3d, 0x39, 0x45, 0x90, 0x13, 0xa9, 0xde},
    {0xc5, 0x98, 0xe9, 0x17, 0xb8, 0x97, 0x9e, 0x03, 0x33, 0x14, 0x13, 0x8f, 0xce, 0x74, 0x0d, 0x54},
    {0x34, 0xba, 0x99, 0x59, 0x9f, 0x70, 0x67, 0xe9, 0x09, 0xee, 0x64, 0x0e, 0xb3, 0xba, 0xfb, 0x75},
    {0xdc, 0xfa, 0x6c, 0x9a, 0x6f, 0x0a, 0x3e, 0xdc, 0x42, 0xf6, 0xae, 0x0d, 0x3c, 0xf7, 0x83, 0xaf},
    {0xea, 0x2d, 0xe3, 0x1f, 0x02, 0x99, 0x1a, 0x7e, 0x6d, 0x93, 0x4c, 0xb5, 0x42, 0xf0, 0x7a, 0x9b},
    {0x53, 0x5e, 0x04, 0xa2, 0x49, 0xa0, 0x73, 0x49, 0x56, 0xb0, 0x88, 0x8c, 0x12, 0xa0, 0xe4, 0x18},
    {0x7d, 0xa7, 0xc5, 0x21, 0x7f, 0x12, 0x95, 0xdd, 0x4d, 0x77, 0x01, 0xfa, 0x71, 0x88, 0x2b, 0x7f},
    {0xdc, 0x9b, 0xc5, 0xa7, 0x6b, 0x84, 0x5c, 0x37, 0x7c, 0xec, 0x05, 0xa1, 0x9f, 0x91, 0x17, 0x3b},
    {0xea, 0xcf, 0xd9, 0x9b, 0x86, 0xcd, 0x2b, 0x43, 0x54, 0x45, 0x82, 0xc6, 0xfe, 0x73, 0x1a, 0x1a},
    {0x77, 0xb8, 0x1b, 0x90, 0xb4, 0xb7, 0x32, 0x76, 0x8f, 0x8a, 0x57, 0x06, 0xc7, 0xdd, 0x08, 0x90},
};

static const uint8_t enclave_signature_expected[ENCLAVE_SIGNATURE_KEY_SLOTS][ENCLAVE_SIGNATURE_SIZE] = {
    {0xe9, 0x9a, 0xce, 0xe9, 0x4d, 0xe1, 0x7f, 0x55, 0xcb, 0x8a, 0xbf, 0xf2, 0x4d, 0x98, 0x27, 0x67},
    {0x34, 0x27, 0xa7, 0xea, 0xa8, 0x98, 0x66, 0x9b, 0xed, 0x43, 0xd3, 0x93, 0xb5, 0xa2, 0x87, 0x8e},
    {0x6c, 0xf3, 0x01, 0x78, 0x53, 0x1b, 0x11, 0x32, 0xf0, 0x27, 0x2f, 0xe3, 0x7d, 0xa6, 0xe2, 0xfd},
    {0xdf, 0x7f, 0x37, 0x65, 0x2f, 0xdb, 0x7c, 0xcf, 0x5b, 0xb6, 0xe4, 0x9c, 0x63, 0xc5, 0x0f, 0xe0},
    {0x9b, 0x5c, 0xee, 0x44, 0x0e, 0xd1, 0xcb, 0x5f, 0x28, 0x9f, 0x12, 0x17, 0x59, 0x64, 0x40, 0xbb},
    {0x94, 0xc2, 0x09, 0x98, 0x62, 0xa7, 0x2b, 0x93, 0xed, 0x36, 0x1f, 0x10, 0xbc, 0x26, 0xbd, 0x41},
    {0x4d, 0xb2, 0x2b, 0xc5, 0x96, 0x47, 0x61, 0xf4, 0x16, 0xe0, 0x81, 0xc3, 0x8e, 0xb9, 0x9c, 0x9b},
    {0xc3, 0x6b, 0x83, 0x55, 0x90, 0x38, 0x0f, 0xea, 0xd1, 0x65, 0xbf, 0x32, 0x4f, 0x8e, 0x62, 0x5b},
    {0x8d, 0x5e, 0x27, 0xbc, 0x14, 0x4f, 0x08, 0xa8, 0x2b, 0x14, 0x89, 0x5e, 0xdf, 0x77, 0x04, 0x31},
    {0xc9, 0xf7, 0x03, 0xf1, 0x6c, 0x65, 0xad, 0x49, 0x74, 0xbe, 0x00, 0x54, 0xfd, 0xa6, 0x9c, 0x32},
};

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
        uint8_t buffer[ENCLAVE_SIGNATURE_SIZE];
        size_t enclave_valid_keys = 0;
        for(size_t key_slot = 0; key_slot < ENCLAVE_SIGNATURE_KEY_SLOTS; key_slot++) {
            if(furi_hal_crypto_store_load_key(key_slot + 1, enclave_signature_iv[key_slot])) {
                if(furi_hal_crypto_encrypt(
                       enclave_signature_input[key_slot], buffer, ENCLAVE_SIGNATURE_SIZE)) {
                    enclave_valid_keys += memcmp(
                                              buffer,
                                              enclave_signature_expected[key_slot],
                                              ENCLAVE_SIGNATURE_SIZE) == 0;
                }
                furi_hal_crypto_store_unload_key(key_slot + 1);
            }
        }
        string_printf(value, "%d", enclave_valid_keys);
        out("enclave_valid_keys", string_get_cstr(value), false, context);
        out("enclave_valid", (enclave_valid_keys == ENCLAVE_SIGNATURE_KEY_SLOTS) ? "true" : "false", true, context);
    } else {
        out("radio_alive", "false", true, context);
    }

    string_clear(value);
}