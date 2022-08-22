#pragma once

#include "core_defines.h"
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <cmsis_compiler.h>

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
