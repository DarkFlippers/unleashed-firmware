#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t start;
    uint32_t end;
    int8_t power_limit;
    uint8_t duty_cycle;
} FuriHalRegionBand;

typedef struct {
    char country_code[4];
    uint16_t bands_count;
    FuriHalRegionBand bands[];
} FuriHalRegion;

/** Initialize region */
void furi_hal_region_init();

/** Get Region Data.
 * 
 * Region data may be allocated in Flash or in RAM.
 * Keep in mind that we don't do memory management on our side.
 *
 * @return     pointer to FuriHalRegion instance (in RAM or Flash, check before freeing on region update)
 */
const FuriHalRegion* furi_hal_region_get();

/** Set device region data
 *
 * @param      region  pointer to the FuriHalRegion
 */
void furi_hal_region_set(FuriHalRegion* region);

/** Check if region data provisioned
 *
 * @return     true if provisioned, false otherwise
 */
bool furi_hal_region_is_provisioned();

/** Get region name
 * 
 * 2 letter Region code according to iso 3166 standard
 * There are 2 extra values that we use in special cases:
 * - "00" - developer edition, unlocked
 * - "WW" - world wide, region provisioned by default
 * - "--" - no provisioned region
 *
 * @return     Pointer to string
 */
const char* furi_hal_region_get_name();

/** Сheck if transmission is allowed on this frequency for your flipper region
 *
 * @param[in]  frequency  The frequency
 * @param      value  frequency in Hz
 *
 * @return     true if allowed
 */
bool furi_hal_region_is_frequency_allowed(uint32_t frequency);

/** Get band data for frequency
 * 
 * 
 *
 * @param[in]  frequency  The frequency
 *
 * @return     { description_of_the_return_value }
 */
const FuriHalRegionBand* furi_hal_region_get_band(uint32_t frequency);

#ifdef __cplusplus
}
#endif
