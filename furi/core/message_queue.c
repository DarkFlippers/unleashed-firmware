#include "kernel.h"
#include "message_queue.h"
#include <FreeRTOS.h>
#include <queue.h>
#include "check.h"

FuriMessageQueue* furi_message_queue_alloc(uint32_t msg_count, uint32_t msg_size) {
    furi_assert((furi_kernel_is_irq_or_masked() == 0U) && (msg_count > 0U) && (msg_size > 0U));

    QueueHandle_t handle = xQueueCreate(msg_count, msg_size);
    furi_check(handle);

    return ((FuriMessageQueue*)handle);
}

void furi_message_queue_free(FuriMessageQueue* instance) {
    furi_assert(furi_kernel_is_irq_or_masked() == 0U);
    furi_assert(instance);

    vQueueDelete((QueueHandle_t)instance);
}

FuriStatus
    furi_message_queue_put(FuriMessageQueue* instance, const void* msg_ptr, uint32_t timeout) {
    QueueHandle_t hQueue = (QueueHandle_t)instance;
    FuriStatus stat;
    BaseType_t yield;

    stat = FuriStatusOk;

    if(furi_kernel_is_irq_or_masked() != 0U) {
        if((hQueue == NULL) || (msg_ptr == NULL) || (timeout != 0U)) {
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
        if((hQueue == NULL) || (msg_ptr == NULL)) {
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

    /* Return execution status */
    return (stat);
}

FuriStatus furi_message_queue_get(FuriMessageQueue* instance, void* msg_ptr, uint32_t timeout) {
    QueueHandle_t hQueue = (QueueHandle_t)instance;
    FuriStatus stat;
    BaseType_t yield;

    stat = FuriStatusOk;

    if(furi_kernel_is_irq_or_masked() != 0U) {
        if((hQueue == NULL) || (msg_ptr == NULL) || (timeout != 0U)) {
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
        if((hQueue == NULL) || (msg_ptr == NULL)) {
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

    /* Return execution status */
    return (stat);
}

uint32_t furi_message_queue_get_capacity(FuriMessageQueue* instance) {
    StaticQueue_t* mq = (StaticQueue_t*)instance;
    uint32_t capacity;

    if(mq == NULL) {
        capacity = 0U;
    } else {
        /* capacity = pxQueue->uxLength */
        capacity = mq->uxDummy4[1];
    }

    /* Return maximum number of messages */
    return (capacity);
}

uint32_t furi_message_queue_get_message_size(FuriMessageQueue* instance) {
    StaticQueue_t* mq = (StaticQueue_t*)instance;
    uint32_t size;

    if(mq == NULL) {
        size = 0U;
    } else {
        /* size = pxQueue->uxItemSize */
        size = mq->uxDummy4[2];
    }

    /* Return maximum message size */
    return (size);
}

uint32_t furi_message_queue_get_count(FuriMessageQueue* instance) {
    QueueHandle_t hQueue = (QueueHandle_t)instance;
    UBaseType_t count;

    if(hQueue == NULL) {
        count = 0U;
    } else if(furi_kernel_is_irq_or_masked() != 0U) {
        count = uxQueueMessagesWaitingFromISR(hQueue);
    } else {
        count = uxQueueMessagesWaiting(hQueue);
    }

    /* Return number of queued messages */
    return ((uint32_t)count);
}

uint32_t furi_message_queue_get_space(FuriMessageQueue* instance) {
    StaticQueue_t* mq = (StaticQueue_t*)instance;
    uint32_t space;
    uint32_t isrm;

    if(mq == NULL) {
        space = 0U;
    } else if(furi_kernel_is_irq_or_masked() != 0U) {
        isrm = taskENTER_CRITICAL_FROM_ISR();

        /* space = pxQueue->uxLength - pxQueue->uxMessagesWaiting; */
        space = mq->uxDummy4[1] - mq->uxDummy4[0];

        taskEXIT_CRITICAL_FROM_ISR(isrm);
    } else {
        space = (uint32_t)uxQueueSpacesAvailable((QueueHandle_t)mq);
    }

    /* Return number of available slots */
    return (space);
}

FuriStatus furi_message_queue_reset(FuriMessageQueue* instance) {
    QueueHandle_t hQueue = (QueueHandle_t)instance;
    FuriStatus stat;

    if(furi_kernel_is_irq_or_masked() != 0U) {
        stat = FuriStatusErrorISR;
    } else if(hQueue == NULL) {
        stat = FuriStatusErrorParameter;
    } else {
        stat = FuriStatusOk;
        (void)xQueueReset(hQueue);
    }

    /* Return execution status */
    return (stat);
}
