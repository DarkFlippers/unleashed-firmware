#include "mutex.h"
#include "check.h"
#include "common_defines.h"

#include <FreeRTOS.h>
#include <semphr.h>

FuriMutex* furi_mutex_alloc(FuriMutexType type) {
    furi_assert(!FURI_IS_IRQ_MODE());

    SemaphoreHandle_t hMutex = NULL;

    if(type == FuriMutexTypeNormal) {
        hMutex = xSemaphoreCreateMutex();
    } else if(type == FuriMutexTypeRecursive) {
        hMutex = xSemaphoreCreateRecursiveMutex();
    } else {
        furi_crash("Programming error");
    }

    furi_check(hMutex != NULL);

    if(type == FuriMutexTypeRecursive) {
        /* Set LSB as 'recursive mutex flag' */
        hMutex = (SemaphoreHandle_t)((uint32_t)hMutex | 1U);
    }

    /* Return mutex ID */
    return ((FuriMutex*)hMutex);
}

void furi_mutex_free(FuriMutex* instance) {
    furi_assert(!FURI_IS_IRQ_MODE());
    furi_assert(instance);

    vSemaphoreDelete((SemaphoreHandle_t)((uint32_t)instance & ~1U));
}

FuriStatus furi_mutex_acquire(FuriMutex* instance, uint32_t timeout) {
    SemaphoreHandle_t hMutex;
    FuriStatus stat;
    uint32_t rmtx;

    hMutex = (SemaphoreHandle_t)((uint32_t)instance & ~1U);

    /* Extract recursive mutex flag */
    rmtx = (uint32_t)instance & 1U;

    stat = FuriStatusOk;

    if(FURI_IS_IRQ_MODE()) {
        stat = FuriStatusErrorISR;
    } else if(hMutex == NULL) {
        stat = FuriStatusErrorParameter;
    } else {
        if(rmtx != 0U) {
            if(xSemaphoreTakeRecursive(hMutex, timeout) != pdPASS) {
                if(timeout != 0U) {
                    stat = FuriStatusErrorTimeout;
                } else {
                    stat = FuriStatusErrorResource;
                }
            }
        } else {
            if(xSemaphoreTake(hMutex, timeout) != pdPASS) {
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

FuriStatus furi_mutex_release(FuriMutex* instance) {
    SemaphoreHandle_t hMutex;
    FuriStatus stat;
    uint32_t rmtx;

    hMutex = (SemaphoreHandle_t)((uint32_t)instance & ~1U);

    /* Extract recursive mutex flag */
    rmtx = (uint32_t)instance & 1U;

    stat = FuriStatusOk;

    if(FURI_IS_IRQ_MODE()) {
        stat = FuriStatusErrorISR;
    } else if(hMutex == NULL) {
        stat = FuriStatusErrorParameter;
    } else {
        if(rmtx != 0U) {
            if(xSemaphoreGiveRecursive(hMutex) != pdPASS) {
                stat = FuriStatusErrorResource;
            }
        } else {
            if(xSemaphoreGive(hMutex) != pdPASS) {
                stat = FuriStatusErrorResource;
            }
        }
    }

    /* Return execution status */
    return (stat);
}

FuriThreadId furi_mutex_get_owner(FuriMutex* instance) {
    SemaphoreHandle_t hMutex;
    FuriThreadId owner;

    hMutex = (SemaphoreHandle_t)((uint32_t)instance & ~1U);

    if((hMutex == NULL)) {
        owner = 0;
    } else if(FURI_IS_IRQ_MODE()) {
        owner = (FuriThreadId)xSemaphoreGetMutexHolderFromISR(hMutex);
    } else {
        owner = (FuriThreadId)xSemaphoreGetMutexHolder(hMutex);
    }

    /* Return owner thread ID */
    return (owner);
}
