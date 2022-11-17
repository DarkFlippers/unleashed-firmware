#pragma once

#include <inttypes.h>

/**
 * @brief Calculates timezone offset in seconds given timezone offset in hours.
 * @param hours timezone offset in hours
 * @return Timezone offset in seconds.
 */
int32_t timezone_offset_from_hours(float hours);

/**
 * @brief Applies timezone offset to a given time.
 * @param time time to apply offset to.
 * @param offset timezone offset in seconds.
 * @return Time with timezone offset applied.
 */
uint64_t timezone_offset_apply(uint64_t time, int32_t offset);
