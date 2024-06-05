#include "semaphore.h"
#include "check.h"
#include "common_defines.h"

#include <FreeRTOS.h>
#include <semphr.h>

struct FuriSemaphore {
    StaticSemaphore_t container;
};

// IMPORTANT: container MUST be the FIRST struct member
static_assert(offsetof(FuriSemaphore, container) == 0);

FuriSemaphore* furi_semaphore_alloc(uint32_t max_count, uint32_t initial_count) {
    furi_check(!FURI_IS_IRQ_MODE());
    furi_check((max_count > 0U) && (initial_count <= max_count));

    FuriSemaphore* instance = malloc(sizeof(FuriSemaphore));

    SemaphoreHandle_t hSemaphore;

    if(max_count == 1U) {
        hSemaphore = xSemaphoreCreateBinaryStatic(&instance->container);
    } else {
        hSemaphore =
            xSemaphoreCreateCountingStatic(max_count, initial_count, &instance->container);
    }

    furi_check(hSemaphore == (SemaphoreHandle_t)instance);

    if(max_count == 1U && initial_count != 0U) {
        furi_check(xSemaphoreGive(hSemaphore) == pdPASS);
    }

    return instance;
}

void furi_semaphore_free(FuriSemaphore* instance) {
    furi_check(instance);
    furi_check(!FURI_IS_IRQ_MODE());

    vSemaphoreDelete((SemaphoreHandle_t)instance);
    free(instance);
}

FuriStatus furi_semaphore_acquire(FuriSemaphore* instance, uint32_t timeout) {
    furi_check(instance);

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

    return stat;
}

FuriStatus furi_semaphore_release(FuriSemaphore* instance) {
    furi_check(instance);

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

    return stat;
}

uint32_t furi_semaphore_get_count(FuriSemaphore* instance) {
    furi_check(instance);

    SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)instance;
    uint32_t count;

    if(FURI_IS_IRQ_MODE()) {
        count = (uint32_t)uxSemaphoreGetCountFromISR(hSemaphore);
    } else {
        count = (uint32_t)uxSemaphoreGetCount(hSemaphore);
    }

    return count;
}
