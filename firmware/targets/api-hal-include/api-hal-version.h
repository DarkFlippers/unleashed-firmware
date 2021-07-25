#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <lib/toolbox/version.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Device Colors */
typedef enum {
    ApiHalVersionColorUnknown=0x00,
    ApiHalVersionColorBlack=0x01,
    ApiHalVersionColorWhite=0x02,
} ApiHalVersionColor;

/** Device Regions */
typedef enum {
    ApiHalVersionRegionUnknown=0x00,
    ApiHalVersionRegionEuRu=0x01,
    ApiHalVersionRegionUsCaAu=0x02,
    ApiHalVersionRegionJp=0x03,
} ApiHalVersionRegion;

/** Init flipper version */
void api_hal_version_init();

/** Check target firmware version */
bool api_hal_version_do_i_belong_here();

/** Get model name */
const char* api_hal_version_get_model_name();

/** Get hardware version */
const uint8_t api_hal_version_get_hw_version();

/** Get hardware target */
const uint8_t api_hal_version_get_hw_target();

/** Get hardware body */
const uint8_t api_hal_version_get_hw_body();

/** Get hardware body color */
const ApiHalVersionColor api_hal_version_get_hw_color();

/** Get hardware connect */
const uint8_t api_hal_version_get_hw_connect();

/** Get hardware region */
const ApiHalVersionRegion api_hal_version_get_hw_region();

/** Get hardware timestamp */
const uint32_t api_hal_version_get_hw_timestamp();

/** Get pointer to target name */
const char* api_hal_version_get_name_ptr();

/** Get pointer to target device name */
const char* api_hal_version_get_device_name_ptr();

/** Get pointer to target ble local device name */
const char* api_hal_version_get_ble_local_device_name_ptr();

const uint8_t* api_hal_version_get_ble_mac();

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
const struct Version* api_hal_version_get_firmware_version(void);

/** Get platform UID size in bytes */
size_t api_hal_version_uid_size();

/** Get const pointer to UID */
const uint8_t* api_hal_version_uid();

#ifdef __cplusplus
}
#endif
