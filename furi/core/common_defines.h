#pragma once

#include "core_defines.h"
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <cmsis_compiler.h>

#ifndef FURI_WARN_UNUSED
#define FURI_WARN_UNUSED __attribute__((warn_unused_result))
#endif

#ifndef FURI_WEAK
#define FURI_WEAK __attribute__((weak))
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

typedef struct {
    uint32_t isrm;
    bool from_isr;
    bool kernel_running;
} __FuriCriticalInfo;

__FuriCriticalInfo __furi_critical_enter(void);

void __furi_critical_exit(__FuriCriticalInfo info);

#ifndef FURI_CRITICAL_ENTER
#define FURI_CRITICAL_ENTER() __FuriCriticalInfo __furi_critical_info = __furi_critical_enter();
#endif

#ifndef FURI_CRITICAL_EXIT
#define FURI_CRITICAL_EXIT() __furi_critical_exit(__furi_critical_info);
#endif

#ifdef __cplusplus
}
#endif
