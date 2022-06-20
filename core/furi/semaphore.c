#include "semaphore.h"
#include "check.h"
#include "common_defines.h"

#include <semphr.h>

osSemaphoreId_t
    osSemaphoreNew(uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t* attr) {
    SemaphoreHandle_t hSemaphore;
    int32_t mem;

    hSemaphore = NULL;

    if((FURI_IS_IRQ_MODE() == 0U) && (max_count > 0U) && (initial_count <= max_count)) {
        mem = -1;

        if(attr != NULL) {
            if((attr->cb_mem != NULL) && (attr->cb_size >= sizeof(StaticSemaphore_t))) {
                /* The memory for control block is provided, use static object */
                mem = 1;
            } else {
                if((attr->cb_mem == NULL) && (attr->cb_size == 0U)) {
                    /* Control block will be allocated from the dynamic pool */
                    mem = 0;
                }
            }
        } else {
            mem = 0;
        }

        if(mem != -1) {
            if(max_count == 1U) {
                if(mem == 1) {
#if(configSUPPORT_STATIC_ALLOCATION == 1)
                    hSemaphore = xSemaphoreCreateBinaryStatic((StaticSemaphore_t*)attr->cb_mem);
#endif
                } else {
#if(configSUPPORT_DYNAMIC_ALLOCATION == 1)
                    hSemaphore = xSemaphoreCreateBinary();
#endif
                }

                if((hSemaphore != NULL) && (initial_count != 0U)) {
                    if(xSemaphoreGive(hSemaphore) != pdPASS) {
                        vSemaphoreDelete(hSemaphore);
                        hSemaphore = NULL;
                    }
                }
            } else {
                if(mem == 1) {
#if(configSUPPORT_STATIC_ALLOCATION == 1)
                    hSemaphore = xSemaphoreCreateCountingStatic(
                        max_count, initial_count, (StaticSemaphore_t*)attr->cb_mem);
#endif
                } else {
#if(configSUPPORT_DYNAMIC_ALLOCATION == 1)
                    hSemaphore = xSemaphoreCreateCounting(max_count, initial_count);
#endif
                }
            }

#if(configQUEUE_REGISTRY_SIZE > 0)
            if(hSemaphore != NULL) {
                if((attr != NULL) && (attr->name != NULL)) {
                    /* Only non-NULL name objects are added to the Queue Registry */
                    vQueueAddToRegistry(hSemaphore, attr->name);
                }
            }
#endif
        }
    }

    /* Return semaphore ID */
    return ((osSemaphoreId_t)hSemaphore);
}

/*
    Acquire a Semaphore token or timeout if no tokens are available.
*/
osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout) {
    SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)semaphore_id;
    osStatus_t stat;
    BaseType_t yield;

    stat = osOK;

    if(hSemaphore == NULL) {
        stat = osErrorParameter;
    } else if(FURI_IS_IRQ_MODE() != 0U) {
        if(timeout != 0U) {
            stat = osErrorParameter;
        } else {
            yield = pdFALSE;

            if(xSemaphoreTakeFromISR(hSemaphore, &yield) != pdPASS) {
                stat = osErrorResource;
            } else {
                portYIELD_FROM_ISR(yield);
            }
        }
    } else {
        if(xSemaphoreTake(hSemaphore, (TickType_t)timeout) != pdPASS) {
            if(timeout != 0U) {
                stat = osErrorTimeout;
            } else {
                stat = osErrorResource;
            }
        }
    }

    /* Return execution status */
    return (stat);
}

/*
    Release a Semaphore token up to the initial maximum count.
*/
osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id) {
    SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)semaphore_id;
    osStatus_t stat;
    BaseType_t yield;

    stat = osOK;

    if(hSemaphore == NULL) {
        stat = osErrorParameter;
    } else if(FURI_IS_IRQ_MODE() != 0U) {
        yield = pdFALSE;

        if(xSemaphoreGiveFromISR(hSemaphore, &yield) != pdTRUE) {
            stat = osErrorResource;
        } else {
            portYIELD_FROM_ISR(yield);
        }
    } else {
        if(xSemaphoreGive(hSemaphore) != pdPASS) {
            stat = osErrorResource;
        }
    }

    /* Return execution status */
    return (stat);
}

/*
    Get current Semaphore token count.
*/
uint32_t osSemaphoreGetCount(osSemaphoreId_t semaphore_id) {
    SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)semaphore_id;
    uint32_t count;

    if(hSemaphore == NULL) {
        count = 0U;
    } else if(FURI_IS_IRQ_MODE() != 0U) {
        count = (uint32_t)uxSemaphoreGetCountFromISR(hSemaphore);
    } else {
        count = (uint32_t)uxSemaphoreGetCount(hSemaphore);
    }

    /* Return number of tokens */
    return (count);
}

/*
    Delete a Semaphore object.
*/
osStatus_t osSemaphoreDelete(osSemaphoreId_t semaphore_id) {
    SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)semaphore_id;
    osStatus_t stat;

#ifndef USE_FreeRTOS_HEAP_1
    if(FURI_IS_IRQ_MODE() != 0U) {
        stat = osErrorISR;
    } else if(hSemaphore == NULL) {
        stat = osErrorParameter;
    } else {
#if(configQUEUE_REGISTRY_SIZE > 0)
        vQueueUnregisterQueue(hSemaphore);
#endif

        stat = osOK;
        vSemaphoreDelete(hSemaphore);
    }
#else
    stat = osError;
#endif

    /* Return execution status */
    return (stat);
}
