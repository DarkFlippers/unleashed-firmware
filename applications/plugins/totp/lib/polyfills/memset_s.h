#pragma once

#include <errno.h>
#include <stdint.h>

#ifndef _RSIZE_T_DECLARED
typedef uint64_t rsize_t;
#define _RSIZE_T_DECLARED
#endif
#ifndef _ERRNOT_DECLARED
typedef int16_t errno_t; //-V677
#define _ERRNOT_DECLARED
#endif

/**
 * @brief Copies the value \p c into each of the first \p n characters of the object pointed to by \p s.
 * @param s pointer to the object to fill
 * @param smax size of the destination object
 * @param c fill byte
 * @param n number of bytes to fill
 * @return \c 0 on success; non-zero otherwise
 */
errno_t memset_s(void* s, rsize_t smax, int c, rsize_t n);