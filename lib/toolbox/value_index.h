#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Get the index of a uint32_t array element which is closest to the given value.
 *
 * Returned index corresponds to the first element found.
 * If no suitable elements were found, the function returns 0.
 *
 * @param   value           value to be searched.
 * @param   values          pointer to the array to perform the search in.
 * @param   values_count    array size.
 *
 * @return value's index.
 */
uint8_t value_index_uint32(const uint32_t value, const uint32_t values[], uint8_t values_count);

/** Get the index of a float array element which is closest to the given value.
 *
 * Returned index corresponds to the first element found.
 * If no suitable elements were found, the function returns 0.
 *
 * @param   value           value to be searched.
 * @param   values          pointer to the array to perform the search in.
 * @param   values_count    array size.
 *
 * @return value's index.
 */
uint8_t value_index_float(const float value, const float values[], uint8_t values_count);

/** Get the index of a bool array element which is equal to the given value.
 *
 * Returned index corresponds to the first element found.
 * If no suitable elements were found, the function returns 0.
 *
 * @param   value           value to be searched.
 * @param   values          pointer to the array to perform the search in.
 * @param   values_count    array size.
 *
 * @return value's index.
 */
uint8_t value_index_bool(const bool value, const bool values[], uint8_t values_count);

#ifdef __cplusplus
}
#endif
