#include "semaphore.h"

#include <FreeRTOS.h>
#include <semphr.h>

#include "check.h"
#include "kernel.h"

#include "event_loop_link_i.h"

// Internal FreeRTOS member names
#define uxMessagesWaiting uxDummy4[0]
#define uxLength          uxDummy4[1]

struct FuriSemaphore {
    StaticSemaphore_t container;
    FuriEventLoopLink event_loop_link;
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

    // Event Loop must be disconnected
    furi_check(!instance->event_loop_link.item_in);
    furi_check(!instance->event_loop_link.item_out);

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

    if(stat == FuriStatusOk) {
        furi_event_loop_link_notify(&instance->event_loop_link, FuriEventLoopEventOut);
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

    if(stat == FuriStatusOk) {
        furi_event_loop_link_notify(&instance->event_loop_link, FuriEventLoopEventIn);
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

uint32_t furi_semaphore_get_space(FuriSemaphore* instance) {
    furi_assert(instance);

    uint32_t space;

    if(furi_kernel_is_irq_or_masked() != 0U) {
        uint32_t isrm = taskENTER_CRITICAL_FROM_ISR();

        space = instance->container.uxLength - instance->container.uxMessagesWaiting;

        taskEXIT_CRITICAL_FROM_ISR(isrm);
    } else {
        space = uxQueueSpacesAvailable((QueueHandle_t)instance);
    }

    return space;
}

static FuriEventLoopLink* furi_semaphore_event_loop_get_link(FuriEventLoopObject* object) {
    FuriSemaphore* instance = object;
    furi_assert(instance);
    return &instance->event_loop_link;
}

static uint32_t
    furi_semaphore_event_loop_get_level(FuriEventLoopObject* object, FuriEventLoopEvent event) {
    FuriSemaphore* instance = object;
    furi_assert(instance);

    if(event == FuriEventLoopEventIn) {
        return furi_semaphore_get_count(instance);
    } else if(event == FuriEventLoopEventOut) {
        return furi_semaphore_get_space(instance);
    } else {
        furi_crash();
    }
}

const FuriEventLoopContract furi_semaphore_event_loop_contract = {
    .get_link = furi_semaphore_event_loop_get_link,
    .get_level = furi_semaphore_event_loop_get_level,
};
