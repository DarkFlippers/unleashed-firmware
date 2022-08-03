#include "furi.h"
#include <string.h>
#include "queue.h"

void furi_init() {
    furi_assert(!furi_is_irq_context());
    furi_assert(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED);

    furi_log_init();
    furi_record_init();
}

void furi_run() {
    furi_assert(!furi_is_irq_context());
    furi_assert(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED);

#if(__ARM_ARCH_7A__ == 0U)
    /* Service Call interrupt might be configured before kernel start     */
    /* and when its priority is lower or equal to BASEPRI, svc intruction */
    /* causes a Hard Fault.                                               */
    NVIC_SetPriority(SVCall_IRQn, 0U);
#endif

    /* Start the kernel scheduler */
    vTaskStartScheduler();
}
