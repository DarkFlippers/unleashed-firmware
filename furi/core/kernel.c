#include "kernel.h"
#include "base.h"
#include "check.h"
#include "common_defines.h"

#include <furi_hal.h>

#include CMSIS_device_header

int32_t furi_kernel_lock() {
    furi_assert(!furi_is_irq_context());

    int32_t lock;

    switch(xTaskGetSchedulerState()) {
    case taskSCHEDULER_SUSPENDED:
        lock = 1;
        break;

    case taskSCHEDULER_RUNNING:
        vTaskSuspendAll();
        lock = 0;
        break;

    case taskSCHEDULER_NOT_STARTED:
    default:
        lock = (int32_t)FuriStatusError;
        break;
    }

    /* Return previous lock state */
    return (lock);
}

int32_t furi_kernel_unlock() {
    furi_assert(!furi_is_irq_context());

    int32_t lock;

    switch(xTaskGetSchedulerState()) {
    case taskSCHEDULER_SUSPENDED:
        lock = 1;

        if(xTaskResumeAll() != pdTRUE) {
            if(xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED) {
                lock = (int32_t)FuriStatusError;
            }
        }
        break;

    case taskSCHEDULER_RUNNING:
        lock = 0;
        break;

    case taskSCHEDULER_NOT_STARTED:
    default:
        lock = (int32_t)FuriStatusError;
        break;
    }

    /* Return previous lock state */
    return (lock);
}

int32_t furi_kernel_restore_lock(int32_t lock) {
    furi_assert(!furi_is_irq_context());

    switch(xTaskGetSchedulerState()) {
    case taskSCHEDULER_SUSPENDED:
    case taskSCHEDULER_RUNNING:
        if(lock == 1) {
            vTaskSuspendAll();
        } else {
            if(lock != 0) {
                lock = (int32_t)FuriStatusError;
            } else {
                if(xTaskResumeAll() != pdTRUE) {
                    if(xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {
                        lock = (int32_t)FuriStatusError;
                    }
                }
            }
        }
        break;

    case taskSCHEDULER_NOT_STARTED:
    default:
        lock = (int32_t)FuriStatusError;
        break;
    }

    /* Return new lock state */
    return (lock);
}

uint32_t furi_kernel_get_tick_frequency() {
    /* Return frequency in hertz */
    return (configTICK_RATE_HZ_RAW);
}

void furi_delay_tick(uint32_t ticks) {
    furi_assert(!furi_is_irq_context());
    if(ticks == 0U) {
        taskYIELD();
    } else {
        vTaskDelay(ticks);
    }
}

FuriStatus furi_delay_until_tick(uint32_t tick) {
    furi_assert(!furi_is_irq_context());

    TickType_t tcnt, delay;
    FuriStatus stat;

    stat = FuriStatusOk;
    tcnt = xTaskGetTickCount();

    /* Determine remaining number of tick to delay */
    delay = (TickType_t)tick - tcnt;

    /* Check if target tick has not expired */
    if((delay != 0U) && (0 == (delay >> (8 * sizeof(TickType_t) - 1)))) {
        if(xTaskDelayUntil(&tcnt, delay) == pdFALSE) {
            /* Did not delay */
            stat = FuriStatusError;
        }
    } else {
        /* No delay or already expired */
        stat = FuriStatusErrorParameter;
    }

    /* Return execution status */
    return (stat);
}

uint32_t furi_get_tick() {
    TickType_t ticks;

    if(furi_is_irq_context() != 0U) {
        ticks = xTaskGetTickCountFromISR();
    } else {
        ticks = xTaskGetTickCount();
    }

    return ticks;
}

uint32_t furi_ms_to_ticks(uint32_t milliseconds) {
#if configTICK_RATE_HZ_RAW == 1000
    return milliseconds;
#else
    return (uint32_t)((float)configTICK_RATE_HZ_RAW) / 1000.0f * (float)milliseconds;
#endif
}

void furi_delay_ms(uint32_t milliseconds) {
    if(!FURI_IS_ISR() && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if(milliseconds > 0 && milliseconds < portMAX_DELAY - 1) {
            milliseconds += 1;
        }
#if configTICK_RATE_HZ_RAW == 1000
        furi_delay_tick(milliseconds);
#else
        furi_delay_tick(furi_ms_to_ticks(milliseconds));
#endif
    } else if(milliseconds > 0) {
        furi_delay_us(milliseconds * 1000);
    }
}

void furi_delay_us(uint32_t microseconds) {
    furi_hal_cortex_delay_us(microseconds);
}
