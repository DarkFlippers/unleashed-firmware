#include "timer.h"
#include "check.h"
#include "memmgr.h"
#include "kernel.h"

#include "core/common_defines.h"
#include <FreeRTOS.h>
#include <timers.h>

typedef struct {
    FuriTimerCallback func;
    void* context;
} TimerCallback_t;

static void TimerCallback(TimerHandle_t hTimer) {
    TimerCallback_t* callb;

    /* Retrieve pointer to callback function and context */
    callb = (TimerCallback_t*)pvTimerGetTimerID(hTimer);

    /* Remove dynamic allocation flag */
    callb = (TimerCallback_t*)((uint32_t)callb & ~1U);

    if(callb != NULL) {
        callb->func(callb->context);
    }
}

FuriTimer* furi_timer_alloc(FuriTimerCallback func, FuriTimerType type, void* context) {
    furi_assert((furi_is_irq_context() == 0U) && (func != NULL));

    TimerHandle_t hTimer;
    TimerCallback_t* callb;
    UBaseType_t reload;
    uint32_t callb_dyn;

    hTimer = NULL;
    callb = NULL;
    callb_dyn = 0U;

    /* Dynamic memory allocation is available: if memory for callback and */
    /* its context is not provided, allocate it from dynamic memory pool */
    if(callb == NULL) {
        callb = (TimerCallback_t*)malloc(sizeof(TimerCallback_t));

        if(callb != NULL) {
            /* Callback memory was allocated from dynamic pool, set flag */
            callb_dyn = 1U;
        }
    }

    if(callb != NULL) {
        callb->func = func;
        callb->context = context;

        if(type == FuriTimerTypeOnce) {
            reload = pdFALSE;
        } else {
            reload = pdTRUE;
        }

        /* Store callback memory dynamic allocation flag */
        callb = (TimerCallback_t*)((uint32_t)callb | callb_dyn);
        // TimerCallback function is always provided as a callback and is used to call application
        // specified function with its context both stored in structure callb.
        hTimer = xTimerCreate(NULL, 1, reload, callb, TimerCallback);
        if((hTimer == NULL) && (callb != NULL) && (callb_dyn == 1U)) {
            /* Failed to create a timer, release allocated resources */
            callb = (TimerCallback_t*)((uint32_t)callb & ~1U);
            free(callb);
        }
    }

    /* Return timer ID */
    return ((FuriTimer*)hTimer);
}

void furi_timer_free(FuriTimer* instance) {
    furi_assert(!furi_is_irq_context());
    furi_assert(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;
    TimerCallback_t* callb;

    callb = (TimerCallback_t*)pvTimerGetTimerID(hTimer);

    furi_check(xTimerDelete(hTimer, portMAX_DELAY) == pdPASS);

    while (furi_timer_is_running(instance)) furi_delay_tick(2);

    if((uint32_t)callb & 1U) {
        /* Callback memory was allocated from dynamic pool, clear flag */
        callb = (TimerCallback_t*)((uint32_t)callb & ~1U);

        /* Return allocated memory to dynamic pool */
        free(callb);
    }
}

FuriStatus furi_timer_start(FuriTimer* instance, uint32_t ticks) {
    furi_assert(!furi_is_irq_context());
    furi_assert(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;
    FuriStatus stat;

    if(xTimerChangePeriod(hTimer, ticks, portMAX_DELAY) == pdPASS) {
        stat = FuriStatusOk;
    } else {
        stat = FuriStatusErrorResource;
    }

    /* Return execution status */
    return (stat);
}

FuriStatus furi_timer_stop(FuriTimer* instance) {
    furi_assert(!furi_is_irq_context());
    furi_assert(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;
    FuriStatus stat;

    if(xTimerIsTimerActive(hTimer) == pdFALSE) {
        stat = FuriStatusErrorResource;
    } else {
        furi_check(xTimerStop(hTimer, portMAX_DELAY) == pdPASS);
        stat = FuriStatusOk;
    }

    /* Return execution status */
    return (stat);
}

uint32_t furi_timer_is_running(FuriTimer* instance) {
    furi_assert(!furi_is_irq_context());
    furi_assert(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;

    /* Return 0: not running, 1: running */
    return (uint32_t)xTimerIsTimerActive(hTimer);
}
