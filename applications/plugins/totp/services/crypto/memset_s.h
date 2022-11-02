#pragma once

#include <errno.h>
#include <stdint.h>
#include <string.h>

#ifndef _RSIZE_T_DECLARED
typedef uint64_t rsize_t;
#define _RSIZE_T_DECLARED
#endif
#ifndef _ERRNOT_DECLARED
typedef int16_t errno_t;
#define _ERRNOT_DECLARED
#endif

errno_t memset_s(void* s, rsize_t smax, int c, rsize_t n);