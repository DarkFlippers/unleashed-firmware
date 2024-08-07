#include "mutex.h"

#include <FreeRTOS.h>
#include <semphr.h>

#include "check.h"

#include "event_loop_link_i.h"

// Internal FreeRTOS member names
#define ucQueueType ucDummy9

struct FuriMutex {
    StaticSemaphore_t container;
    FuriEventLoopLink event_loop_link;
};

// IMPORTANT: container MUST be the FIRST struct member
static_assert(offsetof(FuriMutex, container) == 0);

FuriMutex* furi_mutex_alloc(FuriMutexType type) {
    furi_check(!FURI_IS_IRQ_MODE());

    FuriMutex* instance = malloc(sizeof(FuriMutex));

    SemaphoreHandle_t hMutex;

    if(type == FuriMutexTypeNormal) {
        hMutex = xSemaphoreCreateMutexStatic(&instance->container);
    } else if(type == FuriMutexTypeRecursive) {
        hMutex = xSemaphoreCreateRecursiveMutexStatic(&instance->container);
    } else {
        furi_crash();
    }

    furi_check(hMutex == (SemaphoreHandle_t)instance);

    return instance;
}

void furi_mutex_free(FuriMutex* instance) {
    furi_check(!FURI_IS_IRQ_MODE());
    furi_check(instance);

    // Event Loop must be disconnected
    furi_check(!instance->event_loop_link.item_in);
    furi_check(!instance->event_loop_link.item_out);

    vSemaphoreDelete((SemaphoreHandle_t)instance);
    free(instance);
}

FuriStatus furi_mutex_acquire(FuriMutex* instance, uint32_t timeout) {
    furi_check(instance);

    SemaphoreHandle_t hMutex = (SemaphoreHandle_t)(instance);
    const uint8_t mutex_type = instance->container.ucQueueType;

    FuriStatus stat = FuriStatusOk;

    if(FURI_IS_IRQ_MODE()) {
        stat = FuriStatusErrorISR;

    } else if(mutex_type == queueQUEUE_TYPE_RECURSIVE_MUTEX) {
        if(xSemaphoreTakeRecursive(hMutex, timeout) != pdPASS) {
            if(timeout != 0U) {
                stat = FuriStatusErrorTimeout;
            } else {
                stat = FuriStatusErrorResource;
            }
        }

    } else if(mutex_type == queueQUEUE_TYPE_MUTEX) {
        if(xSemaphoreTake(hMutex, timeout) != pdPASS) {
            if(timeout != 0U) {
                stat = FuriStatusErrorTimeout;
            } else {
                stat = FuriStatusErrorResource;
            }
        }

    } else {
        furi_crash();
    }

    if(stat == FuriStatusOk) {
        furi_event_loop_link_notify(&instance->event_loop_link, FuriEventLoopEventOut);
    }

    return stat;
}

FuriStatus furi_mutex_release(FuriMutex* instance) {
    furi_check(instance);

    SemaphoreHandle_t hMutex = (SemaphoreHandle_t)(instance);
    const uint8_t mutex_type = instance->container.ucQueueType;

    FuriStatus stat = FuriStatusOk;

    if(FURI_IS_IRQ_MODE()) {
        stat = FuriStatusErrorISR;

    } else if(mutex_type == queueQUEUE_TYPE_RECURSIVE_MUTEX) {
        if(xSemaphoreGiveRecursive(hMutex) != pdPASS) {
            stat = FuriStatusErrorResource;
        }

    } else if(mutex_type == queueQUEUE_TYPE_MUTEX) {
        if(xSemaphoreGive(hMutex) != pdPASS) {
            stat = FuriStatusErrorResource;
        }

    } else {
        furi_crash();
    }

    if(stat == FuriStatusOk) {
        furi_event_loop_link_notify(&instance->event_loop_link, FuriEventLoopEventIn);
    }

    return stat;
}

FuriThreadId furi_mutex_get_owner(FuriMutex* instance) {
    furi_check(instance);

    SemaphoreHandle_t hMutex = (SemaphoreHandle_t)instance;

    FuriThreadId owner;

    if(FURI_IS_IRQ_MODE()) {
        owner = (FuriThreadId)xSemaphoreGetMutexHolderFromISR(hMutex);
    } else {
        owner = (FuriThreadId)xSemaphoreGetMutexHolder(hMutex);
    }

    return owner;
}

static FuriEventLoopLink* furi_mutex_event_loop_get_link(FuriEventLoopObject* object) {
    FuriMutex* instance = object;
    furi_assert(instance);
    return &instance->event_loop_link;
}

static uint32_t
    furi_mutex_event_loop_get_level(FuriEventLoopObject* object, FuriEventLoopEvent event) {
    FuriMutex* instance = object;
    furi_assert(instance);

    if(event == FuriEventLoopEventIn || event == FuriEventLoopEventOut) {
        return furi_mutex_get_owner(instance) ? 0 : 1;
    } else {
        furi_crash();
    }
}

const FuriEventLoopContract furi_mutex_event_loop_contract = {
    .get_link = furi_mutex_event_loop_get_link,
    .get_level = furi_mutex_event_loop_get_level,
};
