/**
 * @file furi_hal_light.h
 * Light control HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriHalMpuRegionNULL = 0x00, // region 0 used to protect null pointer dereference
    FuriHalMpuRegionStack = 0x01, // region 1 used to protect stack
    FuriHalMpuRegion2 = 0x02,
    FuriHalMpuRegion3 = 0x03,
    FuriHalMpuRegion4 = 0x04,
    FuriHalMpuRegion5 = 0x05,
    FuriHalMpuRegion6 = 0x06,
    FuriHalMpuRegion7 = 0x07,
} FuriHalMpuRegion;

typedef enum {
    FuriHalMPURegionSize32B = 0x04U,
    FuriHalMPURegionSize64B = 0x05U,
    FuriHalMPURegionSize128B = 0x06U,
    FuriHalMPURegionSize256B = 0x07U,
    FuriHalMPURegionSize512B = 0x08U,
    FuriHalMPURegionSize1KB = 0x09U,
    FuriHalMPURegionSize2KB = 0x0AU,
    FuriHalMPURegionSize4KB = 0x0BU,
    FuriHalMPURegionSize8KB = 0x0CU,
    FuriHalMPURegionSize16KB = 0x0DU,
    FuriHalMPURegionSize32KB = 0x0EU,
    FuriHalMPURegionSize64KB = 0x0FU,
    FuriHalMPURegionSize128KB = 0x10U,
    FuriHalMPURegionSize256KB = 0x11U,
    FuriHalMPURegionSize512KB = 0x12U,
    FuriHalMPURegionSize1MB = 0x13U,
    FuriHalMPURegionSize2MB = 0x14U,
    FuriHalMPURegionSize4MB = 0x15U,
    FuriHalMPURegionSize8MB = 0x16U,
    FuriHalMPURegionSize16MB = 0x17U,
    FuriHalMPURegionSize32MB = 0x18U,
    FuriHalMPURegionSize64MB = 0x19U,
    FuriHalMPURegionSize128MB = 0x1AU,
    FuriHalMPURegionSize256MB = 0x1BU,
    FuriHalMPURegionSize512MB = 0x1CU,
    FuriHalMPURegionSize1GB = 0x1DU,
    FuriHalMPURegionSize2GB = 0x1EU,
    FuriHalMPURegionSize4GB = 0x1FU,
} FuriHalMPURegionSize;

/**
 * @brief Initialize memory protection unit
 */
void furi_hal_mpu_init();

/**
* @brief Enable memory protection unit
*/
void furi_hal_mpu_enable();

/**
* @brief Disable memory protection unit
*/
void furi_hal_mpu_disable();

void furi_hal_mpu_protect_no_access(
    FuriHalMpuRegion region,
    uint32_t address,
    FuriHalMPURegionSize size);

void furi_hal_mpu_protect_read_only(
    FuriHalMpuRegion region,
    uint32_t address,
    FuriHalMPURegionSize size);

void furi_hal_mpu_protect_disable(FuriHalMpuRegion region);

#ifdef __cplusplus
}
#endif
