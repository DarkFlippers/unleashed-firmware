#include "semaphore.h"
#include "check.h"
#include "common_defines.h"

#include <FreeRTOS.h>
#include <semphr.h>

FuriSemaphore* furi_semaphore_alloc(uint32_t max_count, uint32_t initial_count) {
    furi_assert(!FURI_IS_IRQ_MODE());
    furi_assert((max_count > 0U) && (initial_count <= max_count));

    SemaphoreHandle_t hSemaphore = NULL;
    if(max_count == 1U) {
        hSemaphore = xSemaphoreCreateBinary();
        if((hSemaphore != NULL) && (initial_count != 0U)) {
            if(xSemaphoreGive(hSemaphore) != pdPASS) {
                vSemaphoreDelete(hSemaphore);
                hSemaphore = NULL;
            }
        }
    } else {
        hSemaphore = xSemaphoreCreateCounting(max_count, initial_count);
    }

    furi_check(hSemaphore);

    /* Return semaphore ID */
    return ((FuriSemaphore*)hSemaphore);
}

void furi_semaphore_free(FuriSemaphore* instance) {
    furi_assert(instance);
    furi_assert(!FURI_IS_IRQ_MODE());

    SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)instance;

    vSemaphoreDelete(hSemaphore);
}

FuriStatus furi_semaphore_acquire(FuriSemaphore* instance, uint32_t timeout) {
    furi_assert(instance);

    SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)instance;
    FuriStatus stat;
    BaseType_t yield;

    stat = FuriStatusOk;

    if(FURI_IS_IRQ_MODE()) {
        if(timeout != 0U) {
            stat = FuriStatusErrorParameter;
        } else {
            yield = pdFALSE;

            if(xSemaphoreTakeFromISR(hSemaphore, &yield) != pdPASS) {
                stat = FuriStatusErrorResource;
            } else {
                portYIELD_FROM_ISR(yield);
            }
        }
    } else {
        if(xSemaphoreTake(hSemaphore, (TickType_t)timeout) != pdPASS) {
            if(timeout != 0U) {
                stat = FuriStatusErrorTimeout;
            } else {
                stat = FuriStatusErrorResource;
            }
        }
    }

    /* Return execution status */
    return (stat);
}

FuriStatus furi_semaphore_release(FuriSemaphore* instance) {
    furi_assert(instance);

    SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)instance;
    FuriStatus stat;
    BaseType_t yield;

    stat = FuriStatusOk;

    if(FURI_IS_IRQ_MODE()) {
        yield = pdFALSE;

        if(xSemaphoreGiveFromISR(hSemaphore, &yield) != pdTRUE) {
            stat = FuriStatusErrorResource;
        } else {
            portYIELD_FROM_ISR(yield);
        }
    } else {
        if(xSemaphoreGive(hSemaphore) != pdPASS) {
            stat = FuriStatusErrorResource;
        }
    }

    /* Return execution status */
    return (stat);
}

uint32_t furi_semaphore_get_count(FuriSemaphore* instance) {
    furi_assert(instance);

    SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)instance;
    uint32_t count;

    if(FURI_IS_IRQ_MODE()) {
        count = (uint32_t)uxSemaphoreGetCountFromISR(hSemaphore);
    } else {
        count = (uint32_t)uxSemaphoreGetCount(hSemaphore);
    }

    /* Return number of tokens */
    return (count);
}
