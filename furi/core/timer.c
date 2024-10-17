#include "timer.h"
#include "check.h"
#include "kernel.h"

#include <FreeRTOS.h>
#include <event_groups.h>
#include <timers.h>

struct FuriTimer {
    StaticTimer_t container;
    FuriTimerCallback cb_func;
    void* cb_context;
};

// IMPORTANT: container MUST be the FIRST struct member
static_assert(offsetof(FuriTimer, container) == 0);

#define TIMER_DELETED_EVENT (1U << 0)

static void furi_timer_callback(TimerHandle_t hTimer) {
    FuriTimer* instance = pvTimerGetTimerID(hTimer);
    furi_check(instance);
    instance->cb_func(instance->cb_context);
}

static void furi_timer_flush_epilogue(void* context, uint32_t arg) {
    furi_assert(context);
    UNUSED(arg);

    EventGroupHandle_t hEvent = context;

    // See https://github.com/FreeRTOS/FreeRTOS-Kernel/issues/1142
    vTaskSuspendAll();
    xEventGroupSetBits(hEvent, TIMER_DELETED_EVENT);
    (void)xTaskResumeAll();
}

FuriTimer* furi_timer_alloc(FuriTimerCallback func, FuriTimerType type, void* context) {
    furi_check((furi_kernel_is_irq_or_masked() == 0U) && (func != NULL));

    FuriTimer* instance = malloc(sizeof(FuriTimer));

    instance->cb_func = func;
    instance->cb_context = context;

    const UBaseType_t reload = (type == FuriTimerTypeOnce ? pdFALSE : pdTRUE);
    const TimerHandle_t hTimer = xTimerCreateStatic(
        NULL, portMAX_DELAY, reload, instance, furi_timer_callback, &instance->container);

    furi_check(hTimer == (TimerHandle_t)instance);

    return instance;
}

void furi_timer_free(FuriTimer* instance) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;
    furi_check(xTimerDelete(hTimer, portMAX_DELAY) == pdPASS);

    furi_timer_flush();

    free(instance);
}

void furi_timer_flush(void) {
    StaticEventGroup_t event_container = {};
    EventGroupHandle_t hEvent = xEventGroupCreateStatic(&event_container);
    furi_check(
        xTimerPendFunctionCall(furi_timer_flush_epilogue, hEvent, 0, portMAX_DELAY) == pdPASS);

    furi_check(
        xEventGroupWaitBits(hEvent, TIMER_DELETED_EVENT, pdFALSE, pdTRUE, portMAX_DELAY) ==
        TIMER_DELETED_EVENT);
    vEventGroupDelete(hEvent);
}

FuriStatus furi_timer_start(FuriTimer* instance, uint32_t ticks) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);
    furi_check(ticks < portMAX_DELAY);

    TimerHandle_t hTimer = (TimerHandle_t)instance;
    FuriStatus stat;

    if(xTimerChangePeriod(hTimer, ticks, portMAX_DELAY) == pdPASS) {
        stat = FuriStatusOk;
    } else {
        stat = FuriStatusErrorResource;
    }

    return stat;
}

FuriStatus furi_timer_restart(FuriTimer* instance, uint32_t ticks) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);
    furi_check(ticks < portMAX_DELAY);

    TimerHandle_t hTimer = (TimerHandle_t)instance;
    FuriStatus stat;

    if(xTimerChangePeriod(hTimer, ticks, portMAX_DELAY) == pdPASS &&
       xTimerReset(hTimer, portMAX_DELAY) == pdPASS) {
        stat = FuriStatusOk;
    } else {
        stat = FuriStatusErrorResource;
    }

    return stat;
}

FuriStatus furi_timer_stop(FuriTimer* instance) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;

    furi_check(xTimerStop(hTimer, portMAX_DELAY) == pdPASS);

    return FuriStatusOk;
}

uint32_t furi_timer_is_running(FuriTimer* instance) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;

    /* Return 0: not running, 1: running */
    return (uint32_t)xTimerIsTimerActive(hTimer);
}

uint32_t furi_timer_get_expire_time(FuriTimer* instance) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;

    return (uint32_t)xTimerGetExpiryTime(hTimer);
}

void furi_timer_pending_callback(FuriTimerPendigCallback callback, void* context, uint32_t arg) {
    furi_check(callback);

    BaseType_t ret = pdFAIL;
    if(furi_kernel_is_irq_or_masked()) {
        ret = xTimerPendFunctionCallFromISR(callback, context, arg, NULL);
    } else {
        ret = xTimerPendFunctionCall(callback, context, arg, FuriWaitForever);
    }

    furi_check(ret == pdPASS);
}

void furi_timer_set_thread_priority(FuriTimerThreadPriority priority) {
    furi_check(!furi_kernel_is_irq_or_masked());

    TaskHandle_t task_handle = xTimerGetTimerDaemonTaskHandle();
    furi_check(task_handle); // Don't call this method before timer task start

    if(priority == FuriTimerThreadPriorityNormal) {
        vTaskPrioritySet(task_handle, configTIMER_TASK_PRIORITY);
    } else if(priority == FuriTimerThreadPriorityElevated) {
        vTaskPrioritySet(task_handle, configMAX_PRIORITIES - 1);
    } else {
        furi_crash();
    }
}
