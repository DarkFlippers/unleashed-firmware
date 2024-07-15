#include "common_defines.h"

#include <FreeRTOS.h>
#include <task.h>

__FuriCriticalInfo __furi_critical_enter(void) {
    __FuriCriticalInfo info;

    info.isrm = 0;
    info.from_isr = FURI_IS_ISR();
    info.kernel_running = (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING);

    if(info.from_isr) {
        info.isrm = taskENTER_CRITICAL_FROM_ISR();
    } else if(info.kernel_running) {
        taskENTER_CRITICAL();
    } else {
        __disable_irq();
    }

    return info;
}

void __furi_critical_exit(__FuriCriticalInfo info) {
    if(info.from_isr) {
        taskEXIT_CRITICAL_FROM_ISR(info.isrm);
    } else if(info.kernel_running) {
        taskEXIT_CRITICAL();
    } else {
        __enable_irq();
    }
}
