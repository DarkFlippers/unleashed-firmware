#include "event_flag.h"
#include "common_defines.h"
#include "check.h"

#include <event_groups.h>

#define FURI_EVENT_FLAG_MAX_BITS_EVENT_GROUPS 24U
#define FURI_EVENT_FLAG_INVALID_BITS (~((1UL << FURI_EVENT_FLAG_MAX_BITS_EVENT_GROUPS) - 1U))

FuriEventFlag* furi_event_flag_alloc() {
    furi_assert(!FURI_IS_IRQ_MODE());

    EventGroupHandle_t handle = xEventGroupCreate();
    furi_check(handle);

    return ((FuriEventFlag*)handle);
}

void furi_event_flag_free(FuriEventFlag* instance) {
    furi_assert(!FURI_IS_IRQ_MODE());
    vEventGroupDelete((EventGroupHandle_t)instance);
}

uint32_t furi_event_flag_set(FuriEventFlag* instance, uint32_t flags) {
    furi_assert(instance);
    furi_assert((flags & FURI_EVENT_FLAG_INVALID_BITS) == 0U);

    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)instance;
    uint32_t rflags;
    BaseType_t yield;

    if(FURI_IS_IRQ_MODE()) {
        yield = pdFALSE;
        if(xEventGroupSetBitsFromISR(hEventGroup, (EventBits_t)flags, &yield) == pdFAIL) {
            rflags = (uint32_t)FuriFlagErrorResource;
        } else {
            rflags = flags;
            portYIELD_FROM_ISR(yield);
        }
    } else {
        rflags = xEventGroupSetBits(hEventGroup, (EventBits_t)flags);
    }

    /* Return event flags after setting */
    return (rflags);
}

uint32_t furi_event_flag_clear(FuriEventFlag* instance, uint32_t flags) {
    furi_assert(instance);
    furi_assert((flags & FURI_EVENT_FLAG_INVALID_BITS) == 0U);

    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)instance;
    uint32_t rflags;

    if(FURI_IS_IRQ_MODE()) {
        rflags = xEventGroupGetBitsFromISR(hEventGroup);

        if(xEventGroupClearBitsFromISR(hEventGroup, (EventBits_t)flags) == pdFAIL) {
            rflags = (uint32_t)FuriStatusErrorResource;
        } else {
            /* xEventGroupClearBitsFromISR only registers clear operation in the timer command queue. */
            /* Yield is required here otherwise clear operation might not execute in the right order. */
            /* See https://github.com/FreeRTOS/FreeRTOS-Kernel/issues/93 for more info.               */
            portYIELD_FROM_ISR(pdTRUE);
        }
    } else {
        rflags = xEventGroupClearBits(hEventGroup, (EventBits_t)flags);
    }

    /* Return event flags before clearing */
    return (rflags);
}

uint32_t furi_event_flag_get(FuriEventFlag* instance) {
    furi_assert(instance);

    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)instance;
    uint32_t rflags;

    if(FURI_IS_IRQ_MODE()) {
        rflags = xEventGroupGetBitsFromISR(hEventGroup);
    } else {
        rflags = xEventGroupGetBits(hEventGroup);
    }

    /* Return current event flags */
    return (rflags);
}

uint32_t furi_event_flag_wait(
    FuriEventFlag* instance,
    uint32_t flags,
    uint32_t options,
    uint32_t timeout) {
    furi_assert(!FURI_IS_IRQ_MODE());
    furi_assert(instance);
    furi_assert((flags & FURI_EVENT_FLAG_INVALID_BITS) == 0U);

    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)instance;
    BaseType_t wait_all;
    BaseType_t exit_clr;
    uint32_t rflags;

    if(options & FuriFlagWaitAll) {
        wait_all = pdTRUE;
    } else {
        wait_all = pdFAIL;
    }

    if(options & FuriFlagNoClear) {
        exit_clr = pdFAIL;
    } else {
        exit_clr = pdTRUE;
    }

    rflags = xEventGroupWaitBits(
        hEventGroup, (EventBits_t)flags, exit_clr, wait_all, (TickType_t)timeout);

    if(options & FuriFlagWaitAll) {
        if((flags & rflags) != flags) {
            if(timeout > 0U) {
                rflags = (uint32_t)FuriStatusErrorTimeout;
            } else {
                rflags = (uint32_t)FuriStatusErrorResource;
            }
        }
    } else {
        if((flags & rflags) == 0U) {
            if(timeout > 0U) {
                rflags = (uint32_t)FuriStatusErrorTimeout;
            } else {
                rflags = (uint32_t)FuriStatusErrorResource;
            }
        }
    }

    /* Return event flags before clearing */
    return (rflags);
}
