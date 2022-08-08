#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Generates random name
 * @param name buffer to write random name
 * @param max_name_size length of given buffer
 */
void set_random_name(char* name, uint8_t max_name_size);

#ifdef __cplusplus
}
#endif