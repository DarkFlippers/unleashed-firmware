#include "event_flags.h"
#include "common_defines.h"

#include <event_groups.h>

#define MAX_BITS_EVENT_GROUPS 24U
#define EVENT_FLAGS_INVALID_BITS (~((1UL << MAX_BITS_EVENT_GROUPS) - 1U))

osEventFlagsId_t osEventFlagsNew(const osEventFlagsAttr_t* attr) {
    EventGroupHandle_t hEventGroup;
    int32_t mem;

    hEventGroup = NULL;

    if(FURI_IS_IRQ_MODE() == 0U) {
        mem = -1;

        if(attr != NULL) {
            if((attr->cb_mem != NULL) && (attr->cb_size >= sizeof(StaticEventGroup_t))) {
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
            hEventGroup = xEventGroupCreateStatic(attr->cb_mem);
#endif
        } else {
            if(mem == 0) {
#if(configSUPPORT_DYNAMIC_ALLOCATION == 1)
                hEventGroup = xEventGroupCreate();
#endif
            }
        }
    }

    /* Return event flags ID */
    return ((osEventFlagsId_t)hEventGroup);
}

/*
  Set the specified Event Flags.

  Limitations:
  - Event flags are limited to 24 bits.
*/
uint32_t osEventFlagsSet(osEventFlagsId_t ef_id, uint32_t flags) {
    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)ef_id;
    uint32_t rflags;
    BaseType_t yield;

    if((hEventGroup == NULL) || ((flags & EVENT_FLAGS_INVALID_BITS) != 0U)) {
        rflags = (uint32_t)osErrorParameter;
    } else if(FURI_IS_IRQ_MODE() != 0U) {
#if(configUSE_OS2_EVENTFLAGS_FROM_ISR == 0)
        (void)yield;
        /* Enable timers and xTimerPendFunctionCall function to support osEventFlagsSet from ISR */
        rflags = (uint32_t)osErrorResource;
#else
        yield = pdFALSE;

        if(xEventGroupSetBitsFromISR(hEventGroup, (EventBits_t)flags, &yield) == pdFAIL) {
            rflags = (uint32_t)osErrorResource;
        } else {
            rflags = flags;
            portYIELD_FROM_ISR(yield);
        }
#endif
    } else {
        rflags = xEventGroupSetBits(hEventGroup, (EventBits_t)flags);
    }

    /* Return event flags after setting */
    return (rflags);
}

/*
  Clear the specified Event Flags.

  Limitations:
  - Event flags are limited to 24 bits.
*/
uint32_t osEventFlagsClear(osEventFlagsId_t ef_id, uint32_t flags) {
    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)ef_id;
    uint32_t rflags;

    if((hEventGroup == NULL) || ((flags & EVENT_FLAGS_INVALID_BITS) != 0U)) {
        rflags = (uint32_t)osErrorParameter;
    } else if(FURI_IS_IRQ_MODE() != 0U) {
#if(configUSE_OS2_EVENTFLAGS_FROM_ISR == 0)
        /* Enable timers and xTimerPendFunctionCall function to support osEventFlagsSet from ISR */
        rflags = (uint32_t)osErrorResource;
#else
        rflags = xEventGroupGetBitsFromISR(hEventGroup);

        if(xEventGroupClearBitsFromISR(hEventGroup, (EventBits_t)flags) == pdFAIL) {
            rflags = (uint32_t)osErrorResource;
        } else {
            /* xEventGroupClearBitsFromISR only registers clear operation in the timer command queue. */
            /* Yield is required here otherwise clear operation might not execute in the right order. */
            /* See https://github.com/FreeRTOS/FreeRTOS-Kernel/issues/93 for more info.               */
            portYIELD_FROM_ISR(pdTRUE);
        }
#endif
    } else {
        rflags = xEventGroupClearBits(hEventGroup, (EventBits_t)flags);
    }

    /* Return event flags before clearing */
    return (rflags);
}

/*
  Get the current Event Flags.

  Limitations:
  - Event flags are limited to 24 bits.
*/
uint32_t osEventFlagsGet(osEventFlagsId_t ef_id) {
    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)ef_id;
    uint32_t rflags;

    if(ef_id == NULL) {
        rflags = 0U;
    } else if(FURI_IS_IRQ_MODE() != 0U) {
        rflags = xEventGroupGetBitsFromISR(hEventGroup);
    } else {
        rflags = xEventGroupGetBits(hEventGroup);
    }

    /* Return current event flags */
    return (rflags);
}

/*
  Wait for one or more Event Flags to become signaled.

  Limitations:
  - Event flags are limited to 24 bits.
  - osEventFlagsWait cannot be called from an ISR.
*/
uint32_t
    osEventFlagsWait(osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout) {
    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)ef_id;
    BaseType_t wait_all;
    BaseType_t exit_clr;
    uint32_t rflags;

    if((hEventGroup == NULL) || ((flags & EVENT_FLAGS_INVALID_BITS) != 0U)) {
        rflags = (uint32_t)osErrorParameter;
    } else if(FURI_IS_IRQ_MODE() != 0U) {
        rflags = (uint32_t)osErrorISR;
    } else {
        if(options & osFlagsWaitAll) {
            wait_all = pdTRUE;
        } else {
            wait_all = pdFAIL;
        }

        if(options & osFlagsNoClear) {
            exit_clr = pdFAIL;
        } else {
            exit_clr = pdTRUE;
        }

        rflags = xEventGroupWaitBits(
            hEventGroup, (EventBits_t)flags, exit_clr, wait_all, (TickType_t)timeout);

        if(options & osFlagsWaitAll) {
            if((flags & rflags) != flags) {
                if(timeout > 0U) {
                    rflags = (uint32_t)osErrorTimeout;
                } else {
                    rflags = (uint32_t)osErrorResource;
                }
            }
        } else {
            if((flags & rflags) == 0U) {
                if(timeout > 0U) {
                    rflags = (uint32_t)osErrorTimeout;
                } else {
                    rflags = (uint32_t)osErrorResource;
                }
            }
        }
    }

    /* Return event flags before clearing */
    return (rflags);
}

/*
  Delete an Event Flags object.
*/
osStatus_t osEventFlagsDelete(osEventFlagsId_t ef_id) {
    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)ef_id;
    osStatus_t stat;

#ifndef USE_FreeRTOS_HEAP_1
    if(FURI_IS_IRQ_MODE() != 0U) {
        stat = osErrorISR;
    } else if(hEventGroup == NULL) {
        stat = osErrorParameter;
    } else {
        stat = osOK;
        vEventGroupDelete(hEventGroup);
    }
#else
    stat = osError;
#endif

    /* Return execution status */
    return (stat);
}
