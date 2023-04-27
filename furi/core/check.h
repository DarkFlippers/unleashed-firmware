/**
 * @file check.h
 * 
 * Furi crash and assert functions.
 * 
 * The main problem with crashing is that you can't do anything without disturbing registers,
 * and if you disturb registers, you won't be able to see the correct register values in the debugger.
 * 
 * Current solution works around it by passing the message through r12 and doing some magic with registers in crash function.
 * r0-r10 are stored in the ram2 on crash routine start and restored at the end.
 * The only register that is going to be lost is r11.
 * 
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#define FURI_NORETURN [[noreturn]]
#else
#include <stdnoreturn.h>
#define FURI_NORETURN noreturn
#endif

// Flags instead of pointers will save ~4 bytes on furi_assert and furi_check calls.
#define __FURI_ASSERT_MESSAGE_FLAG (0x01)
#define __FURI_CHECK_MESSAGE_FLAG (0x02)

/** Crash system */
FURI_NORETURN void __furi_crash();

/** Halt system */
FURI_NORETURN void __furi_halt();

/** Crash system with message. Show message after reboot. */
#define furi_crash(message)                                   \
    do {                                                      \
        register const void* r12 asm("r12") = (void*)message; \
        asm volatile("sukima%=:" : : "r"(r12));               \
        __furi_crash();                                       \
    } while(0)

/** Halt system with message. */
#define furi_halt(message)                                    \
    do {                                                      \
        register const void* r12 asm("r12") = (void*)message; \
        asm volatile("sukima%=:" : : "r"(r12));               \
        __furi_halt();                                        \
    } while(0)

/** Check condition and crash if check failed */
#define furi_check(__e)                            \
    do {                                           \
        if(!(__e)) {                               \
            furi_crash(__FURI_CHECK_MESSAGE_FLAG); \
        }                                          \
    } while(0)

/** Only in debug build: Assert condition and crash if assert failed  */
#ifdef FURI_DEBUG
#define furi_assert(__e)                            \
    do {                                            \
        if(!(__e)) {                                \
            furi_crash(__FURI_ASSERT_MESSAGE_FLAG); \
        }                                           \
    } while(0)
#else
#define furi_assert(__e) \
    do {                 \
        ((void)(__e));   \
    } while(0)
#endif

#ifdef __cplusplus
}
#endif
