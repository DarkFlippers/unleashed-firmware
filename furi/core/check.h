#pragma once

#ifdef __cplusplus
extern "C" {
#define FURI_NORETURN [[noreturn]]
#else
#include <stdnoreturn.h>
#define FURI_NORETURN noreturn
#endif

/** Check condition and crash if check failed */
#define furi_check(__e) ((__e) ? (void)0 : furi_crash("furi_check failed\r\n"))

/** Only in debug build: Assert condition and crash if assert failed  */
#ifdef FURI_DEBUG
#define furi_assert(__e) ((__e) ? (void)0 : furi_crash("furi_assert failed\r\n"))
#else
#define furi_assert(__e) \
    do {                 \
        ((void)(__e));   \
    } while(0)
#endif

/** Crash system */
FURI_NORETURN void furi_crash(const char* message);

/** Halt system */
FURI_NORETURN void furi_halt(const char* message);

#ifdef __cplusplus
}
#endif
