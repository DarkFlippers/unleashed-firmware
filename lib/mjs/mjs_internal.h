/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_INTERNAL_H_
#define MJS_INTERNAL_H_

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef FAST
#define FAST
#endif

#ifndef STATIC
#define STATIC
#endif

#ifndef ENDL
#define ENDL "\n"
#endif

#ifndef MJS_EXPOSE_PRIVATE
#define MJS_EXPOSE_PRIVATE 1
#endif

#if MJS_EXPOSE_PRIVATE
#define MJS_PRIVATE
#else
#define MJS_PRIVATE static
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#if !defined(WEAK)
#if(defined(__GNUC__) || defined(__TI_COMPILER_VERSION__)) && !defined(_WIN32)
#define WEAK __attribute__((weak))
#else
#define WEAK
#endif
#endif

#ifndef CS_ENABLE_STDIO
#define CS_ENABLE_STDIO 1
#endif

#include "common/cs_dbg.h"
#include "common/cs_file.h"
#include "common/mbuf.h"

#if defined(_WIN32) && _MSC_VER < 1700
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef __int64 int64_t;
typedef unsigned long uintptr_t;
#define STRX(x) #x
#define STR(x) STRX(x)
#define __func__ __FILE__ ":" STR(__LINE__)
// #define snprintf _snprintf
#define vsnprintf _vsnprintf
#define isnan(x) _isnan(x)
#define va_copy(x, y) (x) = (y)
#define CS_DEFINE_DIRENT
#include <windows.h>
#else
#if defined(__unix__) || defined(__APPLE__)
#include <dlfcn.h>
#endif
#endif

/*
 * Number of bytes reserved for the jump offset initially. The most practical
 * value is 1, but for testing it's useful to set it to 0 and to some large
 * value as well (like, 4), to make sure that the code behaves correctly under
 * all circumstances.
 */
#ifndef MJS_INIT_OFFSET_SIZE
#define MJS_INIT_OFFSET_SIZE 1
#endif

#endif /* MJS_INTERNAL_H_ */
