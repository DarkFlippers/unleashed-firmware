#pragma once

#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <cmsis_compiler.h>

#ifndef MAX
#define MAX(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })
#endif

#ifndef MIN
#define MIN(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#endif

#ifndef ROUND_UP_TO
#define ROUND_UP_TO(a, b)       \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a / _b + !!(_a % _b);  \
    })
#endif

#ifndef CLAMP
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#endif

#ifndef COUNT_OF
#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef FURI_SWAP
#define FURI_SWAP(x, y)     \
    do {                    \
        typeof(x) SWAP = x; \
        x = y;              \
        y = SWAP;           \
    } while(0)
#endif

#ifndef PLACE_IN_SECTION
#define PLACE_IN_SECTION(x) __attribute__((section(x)))
#endif

#ifndef ALIGN
#define ALIGN(n) __attribute__((aligned(n)))
#endif

#ifndef __weak
#define __weak __attribute__((weak))
#endif

#ifndef UNUSED
#define UNUSED(X) (void)(X)
#endif

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

#ifndef TOSTRING
#define TOSTRING(x) STRINGIFY(x)
#endif

#ifndef REVERSE_BYTES_U32
#define REVERSE_BYTES_U32(x)                                                        \
    ((((x)&0x000000FF) << 24) | (((x)&0x0000FF00) << 8) | (((x)&0x00FF0000) >> 8) | \
     (((x)&0xFF000000) >> 24))
#endif

#ifndef FURI_BIT
#define FURI_BIT(x, n) (((x) >> (n)) & 1)
#endif

#ifndef FURI_BIT_SET
#define FURI_BIT_SET(x, n) ((x) |= (1 << (n)))
#endif

#ifndef FURI_BIT_CLEAR
#define FURI_BIT_CLEAR(x, n) ((x) &= ~(1 << (n)))
#endif

#ifndef FURI_IS_IRQ_MASKED
#define FURI_IS_IRQ_MASKED() (__get_PRIMASK() != 0U)
#endif

#ifndef FURI_IS_IRQ_MODE
#define FURI_IS_IRQ_MODE() (__get_IPSR() != 0U)
#endif

#ifndef FURI_IS_ISR
#define FURI_IS_ISR() (FURI_IS_IRQ_MODE() || FURI_IS_IRQ_MASKED())
#endif

#ifndef FURI_CRITICAL_ENTER
#define FURI_CRITICAL_ENTER()                                                    \
    uint32_t __isrm = 0;                                                         \
    bool __from_isr = FURI_IS_ISR();                                             \
    bool __kernel_running = (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING); \
    if(__from_isr) {                                                             \
        __isrm = taskENTER_CRITICAL_FROM_ISR();                                  \
    } else if(__kernel_running) {                                                \
        taskENTER_CRITICAL();                                                    \
    } else {                                                                     \
        __disable_irq();                                                         \
    }
#endif

#ifndef FURI_CRITICAL_EXIT
#define FURI_CRITICAL_EXIT()                \
    if(__from_isr) {                        \
        taskEXIT_CRITICAL_FROM_ISR(__isrm); \
    } else if(__kernel_running) {           \
        taskEXIT_CRITICAL();                \
    } else {                                \
        __enable_irq();                     \
    }
#endif

static inline bool furi_is_irq_context() {
    bool irq = false;
    BaseType_t state;

    if(FURI_IS_IRQ_MODE()) {
        /* Called from interrupt context */
        irq = true;
    } else {
        /* Get FreeRTOS scheduler state */
        state = xTaskGetSchedulerState();

        if(state != taskSCHEDULER_NOT_STARTED) {
            /* Scheduler was started */
            if(FURI_IS_IRQ_MASKED()) {
                /* Interrupts are masked */
                irq = true;
            }
        }
    }

    /* Return context, 0: thread context, 1: IRQ context */
    return (irq);
}

#ifdef __cplusplus
}
#endif
