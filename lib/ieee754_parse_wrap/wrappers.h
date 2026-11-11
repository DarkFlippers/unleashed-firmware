#pragma once

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * strtof/strtod are wrapped to a decimal-only parser, so that newlib's
 * arbitrary-precision machinery stays out of flash. Compared to the standard:
 *
 * - no hexadecimal floats ("0x1p3") and no "inf"/"nan" spellings; both are
 *   "no conversion performed", so tail is left at the start of the input
 * - results are within 4 ulp rather than correctly rounded. Integers up to
 *   2^53 and anything strtof narrows to a float are exact
 * - a value within 2 ulp of DBL_MAX saturates to infinity
 *
 * errno is still set to ERANGE on overflow and underflow, and the tail pointer
 * follows the standard in every case.
 */
float __wrap_strtof(const char* in, char** tail);
double __wrap_strtod(const char* in, char** tail);

#ifdef __cplusplus
}
#endif
