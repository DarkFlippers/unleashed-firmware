#pragma once

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

float __wrap_strtof(const char* in, char** tail);
double __wrap_strtod(const char* in, char** tail);

#ifdef __cplusplus
}
#endif
