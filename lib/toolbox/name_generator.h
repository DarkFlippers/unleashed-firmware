#pragma once

#include <stdint.h>
#include <stddef.h>
#include <furi_hal_rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Generates detailed/random name based on furi_hal flags
 *
 * @param      name           buffer to write random name
 * @param      max_name_size  length of given buffer
 * @param[in]  prefix         The prefix of the name
 */
void name_generator_make_auto(char* name, size_t max_name_size, const char* prefix);
void name_generator_make_auto_datetime(
    char* name,
    size_t max_name_size,
    const char* prefix,
    DateTime* custom_time);

// Generate name without prefix in random names
void name_generator_make_auto_basic(char* name, size_t max_name_size, const char* prefix);

/** Generates random name
 *
 * @param      name           buffer to write random name
 * @param      max_name_size  length of given buffer
 * @param[in]  prefix         The prefix of the name
 */
void name_generator_make_random(char* name, size_t max_name_size);
void name_generator_make_random_prefixed(char* name, size_t max_name_size, const char* prefix);

/** Generates detailed name
 *
 * @param      name           buffer to write random name
 * @param      max_name_size  length of given buffer
 * @param[in]  prefix         The prefix of the name
 */
void name_generator_make_detailed(char* name, size_t max_name_size, const char* prefix);
void name_generator_make_detailed_datetime(
    char* name,
    size_t max_name_size,
    const char* prefix,
    DateTime* custom_time);

#ifdef __cplusplus
}
#endif
