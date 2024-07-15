#pragma once

#include <stdint.h>
#include <stddef.h>

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

/** Generates random name
 *
 * @param      name           buffer to write random name
 * @param      max_name_size  length of given buffer
 */
void name_generator_make_random(char* name, size_t max_name_size);

/** Generates detailed name
 *
 * @param      name           buffer to write random name
 * @param      max_name_size  length of given buffer
 * @param[in]  prefix         The prefix of the name
 */
void name_generator_make_detailed(char* name, size_t max_name_size, const char* prefix);

#ifdef __cplusplus
}
#endif
