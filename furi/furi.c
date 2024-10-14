#include "furi.h"

#include "core/thread_i.h"

#include <FreeRTOS.h>
#include <queue.h>

void furi_init(void) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED);

    furi_thread_init();
    furi_log_init();
    furi_record_init();
}

void furi_run(void) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED);

    /* Start the kernel scheduler */
    vTaskStartScheduler();
}

void furi_background(void) {
    furi_thread_scrub();
}
