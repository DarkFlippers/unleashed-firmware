#pragma once

#ifdef __cplusplus
extern "C" {
#define FURI_NORETURN [[noreturn]]
#else
#include <stdnoreturn.h>
#define FURI_NORETURN noreturn
#endif

/** Pointer to pass message to __furi_crash and __furi_halt */
extern const char* __furi_check_message;

/** Crash system */
FURI_NORETURN void __furi_crash();

/** Halt system */
FURI_NORETURN void __furi_halt();

/** Crash system with message. Show message after reboot. */
#define furi_crash(message)             \
    do {                                \
        __furi_check_message = message; \
        __furi_crash();                 \
    } while(0)

/** Halt system with message. */
#define furi_halt(message)              \
    do {                                \
        __furi_check_message = message; \
        __furi_halt();                  \
    } while(0)

/** Check condition and crash if check failed */
#define furi_check(__e)                             \
    do {                                            \
        if ((__e) == 0) {                           \
            furi_crash("furi_check failed\r\n");    \
        }                                           \
    } while(0)

/** Only in debug build: Assert condition and crash if assert failed  */
#ifdef FURI_DEBUG
#define furi_assert(__e)                            \
    do {                                            \
        if ((__e) == 0) {                           \
            furi_crash("furi_assert failed\r\n");   \
        }                                           \
    } while(0)
#else
#define furi_assert(__e)    \
    do {                    \
        ((void)(__e));      \
    } while(0)
#endif

#ifdef __cplusplus
}
#endif
