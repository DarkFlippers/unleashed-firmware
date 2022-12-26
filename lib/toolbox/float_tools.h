#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/** Compare two floating point numbers
 * @param a         First number to compare
 * @param b         Second number to compare
 *
 * @return          bool true if a equals b, false otherwise
 */
bool float_is_equal(float a, float b);

#ifdef __cplusplus
}
#endif
