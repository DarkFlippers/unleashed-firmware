#include "furi.h"

#include <FreeRTOS.h>
#include <queue.h>

void furi_init(void) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED);

    furi_log_init();
    furi_record_init();
}

void furi_run(void) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED);

    /* Start the kernel scheduler */
    vTaskStartScheduler();
}
