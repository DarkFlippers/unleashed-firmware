#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Find how to how get function's pretty name
#ifndef __FURI_CHECK_FUNC
// Use g++'s demangled names in C++
#if defined __cplusplus && defined __GNUC__
#define __FURI_CHECK_FUNC __PRETTY_FUNCTION__

// C99 requires the use of __func__
#elif __STDC_VERSION__ >= 199901L
#define __FURI_CHECK_FUNC __func__

// Older versions of gcc don't have __func__ but can use __FUNCTION__
#elif __GNUC__ >= 2
#define __FURI_CHECK_FUNC __FUNCTION__

// failed to detect __func__ support
#else
#define __FURI_CHECK_FUNC ((char*)0)
#endif
#endif
// !__FURI_CHECK_FUNC

// We have two levels of assertion
// One - furi_check, which always runs, the only difference is in the level of debug information
// The second is furi_assert, which doesn't compile in release mode
#define furi_check(__e) ((__e) ? (void)0 : __furi_check())

#ifdef NDEBUG
#define furi_assert(__e) ((void)0)
#else
#define furi_assert(__e) ((__e) ? (void)0 : __furi_check())
#endif
// !NDEBUG

void __furi_check(void);
void __furi_check_debug(const char* file, int line, const char* function, const char* condition);

#ifdef __cplusplus
}
#endif
