#include "message_queue_i.h"

#include <FreeRTOS.h>
#include <queue.h>

#include "kernel.h"
#include "check.h"

// Internal FreeRTOS member names
#define uxMessagesWaiting uxDummy4[0]
#define uxLength          uxDummy4[1]
#define uxItemSize        uxDummy4[2]

struct FuriMessageQueue {
    StaticQueue_t container;

    // Event Loop Link
    FuriEventLoopLink event_loop_link;

    uint8_t buffer[];
};

// IMPORTANT: container MUST be the FIRST struct member
static_assert(offsetof(FuriMessageQueue, container) == 0);
// IMPORTANT: buffer MUST be the LAST struct member
static_assert(offsetof(FuriMessageQueue, buffer) == sizeof(FuriMessageQueue));

FuriMessageQueue* furi_message_queue_alloc(uint32_t msg_count, uint32_t msg_size) {
    furi_check((furi_kernel_is_irq_or_masked() == 0U) && (msg_count > 0U) && (msg_size > 0U));

    FuriMessageQueue* instance = malloc(sizeof(FuriMessageQueue) + msg_count * msg_size);

    // 3 things happens here:
    // - create queue
    // - check results
    // - ensure that queue container is first in the FuriMessageQueue structure
    //
    // As a bonus it guarantees that FuriMessageQueue* can be casted into StaticQueue_t* or QueueHandle_t.
    furi_check(
        xQueueCreateStatic(msg_count, msg_size, instance->buffer, &instance->container) ==
        (void*)instance);

    return instance;
}

void furi_message_queue_free(FuriMessageQueue* instance) {
    furi_check(furi_kernel_is_irq_or_masked() == 0U);
    furi_check(instance);

    // Event Loop must be disconnected
    furi_check(!instance->event_loop_link.item_in);
    furi_check(!instance->event_loop_link.item_out);

    vQueueDelete((QueueHandle_t)instance);
    free(instance);
}

FuriStatus
    furi_message_queue_put(FuriMessageQueue* instance, const void* msg_ptr, uint32_t timeout) {
    furi_check(instance);

    QueueHandle_t hQueue = (QueueHandle_t)instance;
    FuriStatus stat;
    BaseType_t yield;

    stat = FuriStatusOk;

    if(furi_kernel_is_irq_or_masked() != 0U) {
        if((msg_ptr == NULL) || (timeout != 0U)) {
            stat = FuriStatusErrorParameter;
        } else {
            yield = pdFALSE;

            if(xQueueSendToBackFromISR(hQueue, msg_ptr, &yield) != pdTRUE) {
                stat = FuriStatusErrorResource;
            } else {
                portYIELD_FROM_ISR(yield);
            }
        }
    } else {
        if(msg_ptr == NULL) {
            stat = FuriStatusErrorParameter;
        } else {
            if(xQueueSendToBack(hQueue, msg_ptr, (TickType_t)timeout) != pdPASS) {
                if(timeout != 0U) {
                    stat = FuriStatusErrorTimeout;
                } else {
                    stat = FuriStatusErrorResource;
                }
            }
        }
    }

    if(stat == FuriStatusOk) {
        furi_event_loop_link_notify(&instance->event_loop_link, FuriEventLoopEventIn);
    }

    /* Return execution status */
    return stat;
}

FuriStatus furi_message_queue_get(FuriMessageQueue* instance, void* msg_ptr, uint32_t timeout) {
    furi_check(instance);

    QueueHandle_t hQueue = (QueueHandle_t)instance;
    FuriStatus stat;
    BaseType_t yield;

    stat = FuriStatusOk;

    if(furi_kernel_is_irq_or_masked() != 0U) {
        if((msg_ptr == NULL) || (timeout != 0U)) {
            stat = FuriStatusErrorParameter;
        } else {
            yield = pdFALSE;

            if(xQueueReceiveFromISR(hQueue, msg_ptr, &yield) != pdPASS) {
                stat = FuriStatusErrorResource;
            } else {
                portYIELD_FROM_ISR(yield);
            }
        }
    } else {
        if(msg_ptr == NULL) {
            stat = FuriStatusErrorParameter;
        } else {
            if(xQueueReceive(hQueue, msg_ptr, (TickType_t)timeout) != pdPASS) {
                if(timeout != 0U) {
                    stat = FuriStatusErrorTimeout;
                } else {
                    stat = FuriStatusErrorResource;
                }
            }
        }
    }

    if(stat == FuriStatusOk) {
        furi_event_loop_link_notify(&instance->event_loop_link, FuriEventLoopEventOut);
    }

    return stat;
}

uint32_t furi_message_queue_get_capacity(FuriMessageQueue* instance) {
    furi_check(instance);

    return instance->container.uxLength;
}

uint32_t furi_message_queue_get_message_size(FuriMessageQueue* instance) {
    furi_check(instance);

    return instance->container.uxItemSize;
}

uint32_t furi_message_queue_get_count(FuriMessageQueue* instance) {
    furi_check(instance);

    QueueHandle_t hQueue = (QueueHandle_t)instance;
    UBaseType_t count;

    if(furi_kernel_is_irq_or_masked() != 0U) {
        count = uxQueueMessagesWaitingFromISR(hQueue);
    } else {
        count = uxQueueMessagesWaiting(hQueue);
    }

    return (uint32_t)count;
}

uint32_t furi_message_queue_get_space(FuriMessageQueue* instance) {
    furi_check(instance);

    uint32_t space;
    uint32_t isrm;

    if(furi_kernel_is_irq_or_masked() != 0U) {
        isrm = taskENTER_CRITICAL_FROM_ISR();

        space = instance->container.uxLength - instance->container.uxMessagesWaiting;

        taskEXIT_CRITICAL_FROM_ISR(isrm);
    } else {
        space = (uint32_t)uxQueueSpacesAvailable((QueueHandle_t)instance);
    }

    return space;
}

FuriStatus furi_message_queue_reset(FuriMessageQueue* instance) {
    furi_check(instance);

    QueueHandle_t hQueue = (QueueHandle_t)instance;
    FuriStatus stat;

    if(furi_kernel_is_irq_or_masked() != 0U) {
        stat = FuriStatusErrorISR;
    } else {
        stat = FuriStatusOk;
        (void)xQueueReset(hQueue);
    }

    if(stat == FuriStatusOk) {
        furi_event_loop_link_notify(&instance->event_loop_link, FuriEventLoopEventOut);
    }

    /* Return execution status */
    return stat;
}

static FuriEventLoopLink* furi_message_queue_event_loop_get_link(void* object) {
    FuriMessageQueue* instance = object;
    furi_assert(instance);
    return &instance->event_loop_link;
}

static uint32_t furi_message_queue_event_loop_get_level(void* object, FuriEventLoopEvent event) {
    FuriMessageQueue* instance = object;
    furi_assert(instance);

    if(event == FuriEventLoopEventIn) {
        return furi_message_queue_get_count(instance);
    } else if(event == FuriEventLoopEventOut) {
        return furi_message_queue_get_space(instance);
    } else {
        furi_crash();
    }
}

const FuriEventLoopContract furi_message_queue_event_loop_contract = {
    .get_link = furi_message_queue_event_loop_get_link,
    .get_level = furi_message_queue_event_loop_get_level,
};
