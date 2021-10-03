/**
 * @file furi-hal-version.h
 * Version HAL API
 */

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
    FuriHalVersionColorUnknown=0x00,
    FuriHalVersionColorBlack=0x01,
    FuriHalVersionColorWhite=0x02,
} FuriHalVersionColor;

/** Device Regions */
typedef enum {
    FuriHalVersionRegionUnknown=0x00,
    FuriHalVersionRegionEuRu=0x01,
    FuriHalVersionRegionUsCaAu=0x02,
    FuriHalVersionRegionJp=0x03,
} FuriHalVersionRegion;

/** Init flipper version
 */
void furi_hal_version_init();

/** Check target firmware version
 *
 * @return     true if target and real matches
 */
bool furi_hal_version_do_i_belong_here();

/** Get model name
 *
 * @return     model name C-string
 */
const char* furi_hal_version_get_model_name();

/** Get hardware version
 *
 * @return     Hardware Version
 */
const uint8_t furi_hal_version_get_hw_version();

/** Get hardware target
 *
 * @return     Hardware Target
 */
const uint8_t furi_hal_version_get_hw_target();

/** Get hardware body
 *
 * @return     Hardware Body
 */
const uint8_t furi_hal_version_get_hw_body();

/** Get hardware body color
 *
 * @return     Hardware Color
 */
const FuriHalVersionColor furi_hal_version_get_hw_color();

/** Get hardware connect
 *
 * @return     Hardware Interconnect
 */
const uint8_t furi_hal_version_get_hw_connect();

/** Get hardware region
 *
 * @return     Hardware Region
 */
const FuriHalVersionRegion furi_hal_version_get_hw_region();

/** Get hardware timestamp
 *
 * @return     Hardware Manufacture timestamp
 */
const uint32_t furi_hal_version_get_hw_timestamp();

/** Get pointer to target name
 *
 * @return     Hardware Name C-string
 */
const char* furi_hal_version_get_name_ptr();

/** Get pointer to target device name
 *
 * @return     Hardware Device Name C-string
 */
const char* furi_hal_version_get_device_name_ptr();

/** Get pointer to target ble local device name
 *
 * @return     Ble Device Name C-string
 */
const char* furi_hal_version_get_ble_local_device_name_ptr();

/** Get BLE MAC address
 *
 * @return     pointer to BLE MAC address
 */
const uint8_t* furi_hal_version_get_ble_mac();

/** Get address of version structure of bootloader, stored in chip flash.
 *
 * @return     Address of boot version structure.
 */
const struct Version* furi_hal_version_get_boot_version();

/** Get address of version structure of firmware.
 *
 * @return     Address of firmware version structure.
 */
const struct Version* furi_hal_version_get_firmware_version();

/** Get platform UID size in bytes
 *
 * @return     UID size in bytes
 */
size_t furi_hal_version_uid_size();

/** Get const pointer to UID
 *
 * @return     pointer to UID
 */
const uint8_t* furi_hal_version_uid();

#ifdef __cplusplus
}
#endif
