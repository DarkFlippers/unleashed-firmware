#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <version.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Init flipper version */
void api_hal_version_init();

/** Check target firmware version */
bool api_hal_version_do_i_belong_here();

/** Get hardware version */
const uint8_t api_hal_version_get_hw_version();

/** Get hardware target */
const uint8_t api_hal_version_get_hw_target();

/** Get hardware body */
const uint8_t api_hal_version_get_hw_body();

/** Get hardware connect */
const uint8_t api_hal_version_get_hw_connect();

/** Get hardware timestamp */
const uint32_t api_hal_version_get_hw_timestamp();

/** Get pointer to target name */
const char* api_hal_version_get_name_ptr();

/** Get pointer to target device name */
const char* api_hal_version_get_device_name_ptr();

/** Get pointer to target ble local device name */
const char* api_hal_version_get_ble_local_device_name_ptr();

/**
 * Get address of version structure of bootloader, stored in chip flash.
 *
 * @return Address of boot version structure.
 */
const struct Version* api_hal_version_get_boot_version(void);

/**
 * Get address of version structure of firmware.
 *
 * @return Address of firmware version structure.
 */
const struct Version* api_hal_version_get_fw_version(void);

#ifdef __cplusplus
}
#endif
