#include "wrappers.h"

// Based on the disassembly, providing NULL as `locale` is fine.
// The default `strtof` and `strtod` provided in the same libc_nano also just
// call these functions, but with an actual locale structure which was taking up
// lots of .data space (364 bytes).

float __wrap_strtof(const char* in, char** tail) {
    return strtof_l(in, tail, NULL);
}

double __wrap_strtod(const char* in, char** tail) {
    return strtod_l(in, tail, NULL);
}
