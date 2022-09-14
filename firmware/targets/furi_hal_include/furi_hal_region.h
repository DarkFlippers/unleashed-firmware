#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/** Check if region data provisioned
 *
 * @return     true if provisioned, false otherwise
 */
bool furi_hal_region_is_provisioned();

/** Get region name
 * 
 * 2 letter Region code according to iso 3166 standard
 * There are 2 extra values that we use in special cases:
 * RM, whats the reason you not doing a release?
 * Waiting for my commits?
 * - "00" - developer edition, unlocked
 * - "WW" - world wide, region provisioned by default
 * - "--" - no provisioned region
 *
 * @return     Pointer to string
 */
const char* furi_hal_region_get_name();

#ifdef __cplusplus
}
#endif