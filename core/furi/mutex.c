#include "mutex.h"
#include "check.h"
#include "common_defines.h"

#include <semphr.h>

osMutexId_t osMutexNew(const osMutexAttr_t* attr) {
    SemaphoreHandle_t hMutex;
    uint32_t type;
    uint32_t rmtx;
    int32_t mem;

    hMutex = NULL;

    if(FURI_IS_IRQ_MODE() == 0U) {
        if(attr != NULL) {
            type = attr->attr_bits;
        } else {
            type = 0U;
        }

        if((type & osMutexRecursive) == osMutexRecursive) {
            rmtx = 1U;
        } else {
            rmtx = 0U;
        }

        if((type & osMutexRobust) != osMutexRobust) {
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

            if(mem == 1) {
#if(configSUPPORT_STATIC_ALLOCATION == 1)
                if(rmtx != 0U) {
#if(configUSE_RECURSIVE_MUTEXES == 1)
                    hMutex = xSemaphoreCreateRecursiveMutexStatic(attr->cb_mem);
#endif
                } else {
                    hMutex = xSemaphoreCreateMutexStatic(attr->cb_mem);
                }
#endif
            } else {
                if(mem == 0) {
#if(configSUPPORT_DYNAMIC_ALLOCATION == 1)
                    if(rmtx != 0U) {
#if(configUSE_RECURSIVE_MUTEXES == 1)
                        hMutex = xSemaphoreCreateRecursiveMutex();
#endif
                    } else {
                        hMutex = xSemaphoreCreateMutex();
                    }
#endif
                }
            }

#if(configQUEUE_REGISTRY_SIZE > 0)
            if(hMutex != NULL) {
                if((attr != NULL) && (attr->name != NULL)) {
                    /* Only non-NULL name objects are added to the Queue Registry */
                    vQueueAddToRegistry(hMutex, attr->name);
                }
            }
#endif

            if((hMutex != NULL) && (rmtx != 0U)) {
                /* Set LSB as 'recursive mutex flag' */
                hMutex = (SemaphoreHandle_t)((uint32_t)hMutex | 1U);
            }
        }
    }

    /* Return mutex ID */
    return ((osMutexId_t)hMutex);
}

/*
    Acquire a Mutex or timeout if it is locked.
*/
osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout) {
    SemaphoreHandle_t hMutex;
    osStatus_t stat;
    uint32_t rmtx;

    hMutex = (SemaphoreHandle_t)((uint32_t)mutex_id & ~1U);

    /* Extract recursive mutex flag */
    rmtx = (uint32_t)mutex_id & 1U;

    stat = osOK;

    if(FURI_IS_IRQ_MODE() != 0U) {
        stat = osErrorISR;
    } else if(hMutex == NULL) {
        stat = osErrorParameter;
    } else {
        if(rmtx != 0U) {
#if(configUSE_RECURSIVE_MUTEXES == 1)
            if(xSemaphoreTakeRecursive(hMutex, timeout) != pdPASS) {
                if(timeout != 0U) {
                    stat = osErrorTimeout;
                } else {
                    stat = osErrorResource;
                }
            }
#endif
        } else {
            if(xSemaphoreTake(hMutex, timeout) != pdPASS) {
                if(timeout != 0U) {
                    stat = osErrorTimeout;
                } else {
                    stat = osErrorResource;
                }
            }
        }
    }

    /* Return execution status */
    return (stat);
}

/*
    Release a Mutex that was acquired by osMutexAcquire.
*/
osStatus_t osMutexRelease(osMutexId_t mutex_id) {
    SemaphoreHandle_t hMutex;
    osStatus_t stat;
    uint32_t rmtx;

    hMutex = (SemaphoreHandle_t)((uint32_t)mutex_id & ~1U);

    /* Extract recursive mutex flag */
    rmtx = (uint32_t)mutex_id & 1U;

    stat = osOK;

    if(FURI_IS_IRQ_MODE() != 0U) {
        stat = osErrorISR;
    } else if(hMutex == NULL) {
        stat = osErrorParameter;
    } else {
        if(rmtx != 0U) {
#if(configUSE_RECURSIVE_MUTEXES == 1)
            if(xSemaphoreGiveRecursive(hMutex) != pdPASS) {
                stat = osErrorResource;
            }
#endif
        } else {
            if(xSemaphoreGive(hMutex) != pdPASS) {
                stat = osErrorResource;
            }
        }
    }

    /* Return execution status */
    return (stat);
}

/*
    Get Thread which owns a Mutex object.
*/
FuriThreadId osMutexGetOwner(osMutexId_t mutex_id) {
    SemaphoreHandle_t hMutex;
    FuriThreadId owner;

    hMutex = (SemaphoreHandle_t)((uint32_t)mutex_id & ~1U);

    if((FURI_IS_IRQ_MODE() != 0U) || (hMutex == NULL)) {
        owner = 0;
    } else {
        owner = (FuriThreadId)xSemaphoreGetMutexHolder(hMutex);
    }

    /* Return owner thread ID */
    return (owner);
}

/*
    Delete a Mutex object.
*/
osStatus_t osMutexDelete(osMutexId_t mutex_id) {
    osStatus_t stat;
#ifndef USE_FreeRTOS_HEAP_1
    SemaphoreHandle_t hMutex;

    hMutex = (SemaphoreHandle_t)((uint32_t)mutex_id & ~1U);

    if(FURI_IS_IRQ_MODE() != 0U) {
        stat = osErrorISR;
    } else if(hMutex == NULL) {
        stat = osErrorParameter;
    } else {
#if(configQUEUE_REGISTRY_SIZE > 0)
        vQueueUnregisterQueue(hMutex);
#endif
        stat = osOK;
        vSemaphoreDelete(hMutex);
    }
#else
    stat = osError;
#endif

    /* Return execution status */
    return (stat);
}
