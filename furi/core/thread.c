#include "thread.h"
#include "kernel.h"
#include "memmgr.h"
#include "memmgr_heap.h"
#include "check.h"
#include "common_defines.h"
#include "mutex.h"
#include "string.h"

#include <task.h>
#include "log.h"
#include <furi_hal_rtc.h>
#include <furi_hal_console.h>

#define TAG "FuriThread"

#define THREAD_NOTIFY_INDEX 1 // Index 0 is used for stream buffers

typedef struct FuriThreadStdout FuriThreadStdout;

struct FuriThreadStdout {
    FuriThreadStdoutWriteCallback write_callback;
    FuriString* buffer;
};

struct FuriThread {
    FuriThreadState state;
    int32_t ret;

    FuriThreadCallback callback;
    void* context;

    FuriThreadStateCallback state_callback;
    void* state_context;

    char* name;
    char* appid;

    FuriThreadPriority priority;

    TaskHandle_t task_handle;
    size_t heap_size;

    FuriThreadStdout output;

    // Keep all non-alignable byte types in one place,
    // this ensures that the size of this structure is minimal
    bool is_service;
    bool heap_trace_enabled;

    configSTACK_DEPTH_TYPE stack_size;
};

static size_t __furi_thread_stdout_write(FuriThread* thread, const char* data, size_t size);
static int32_t __furi_thread_stdout_flush(FuriThread* thread);

/** Catch threads that are trying to exit wrong way */
__attribute__((__noreturn__)) void furi_thread_catch() { //-V1082
    // If you're here it means you're probably doing something wrong
    // with critical sections or with scheduler state
    asm volatile("nop"); // extra magic
    furi_crash("You are doing it wrong"); //-V779
    __builtin_unreachable();
}

static void furi_thread_set_state(FuriThread* thread, FuriThreadState state) {
    furi_assert(thread);
    thread->state = state;
    if(thread->state_callback) {
        thread->state_callback(state, thread->state_context);
    }
}

static void furi_thread_body(void* context) {
    furi_assert(context);
    FuriThread* thread = context;

    // store thread instance to thread local storage
    furi_assert(pvTaskGetThreadLocalStoragePointer(NULL, 0) == NULL);
    vTaskSetThreadLocalStoragePointer(NULL, 0, thread);

    furi_assert(thread->state == FuriThreadStateStarting);
    furi_thread_set_state(thread, FuriThreadStateRunning);

    TaskHandle_t task_handle = xTaskGetCurrentTaskHandle();
    if(thread->heap_trace_enabled == true) {
        memmgr_heap_enable_thread_trace((FuriThreadId)task_handle);
    }

    thread->ret = thread->callback(thread->context);

    if(thread->heap_trace_enabled == true) {
        furi_delay_ms(33);
        thread->heap_size = memmgr_heap_get_thread_memory((FuriThreadId)task_handle);
        furi_log_print_format(
            thread->heap_size ? FuriLogLevelError : FuriLogLevelInfo,
            TAG,
            "%s allocation balance: %zu",
            thread->name ? thread->name : "Thread",
            thread->heap_size);
        memmgr_heap_disable_thread_trace((FuriThreadId)task_handle);
    }

    furi_assert(thread->state == FuriThreadStateRunning);

    if(thread->is_service) {
        FURI_LOG_W(
            TAG,
            "%s service thread TCB memory will not be reclaimed",
            thread->name ? thread->name : "<unknown service>");
    }

    // flush stdout
    __furi_thread_stdout_flush(thread);

    furi_thread_set_state(thread, FuriThreadStateStopped);

    vTaskDelete(NULL);
    furi_thread_catch();
}

FuriThread* furi_thread_alloc() {
    FuriThread* thread = malloc(sizeof(FuriThread));
    thread->output.buffer = furi_string_alloc();
    thread->is_service = false;

    FuriThread* parent = NULL;
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        // TLS is not available, if we called not from thread context
        parent = pvTaskGetThreadLocalStoragePointer(NULL, 0);

        if(parent && parent->appid) {
            furi_thread_set_appid(thread, parent->appid);
        } else {
            furi_thread_set_appid(thread, "unknown");
        }
    } else {
        // if scheduler is not started, we are starting driver thread
        furi_thread_set_appid(thread, "driver");
    }

    FuriHalRtcHeapTrackMode mode = furi_hal_rtc_get_heap_track_mode();
    if(mode == FuriHalRtcHeapTrackModeAll) {
        thread->heap_trace_enabled = true;
    } else if(mode == FuriHalRtcHeapTrackModeTree && furi_thread_get_current_id()) {
        if(parent) thread->heap_trace_enabled = parent->heap_trace_enabled;
    } else {
        thread->heap_trace_enabled = false;
    }

    return thread;
}

FuriThread* furi_thread_alloc_ex(
    const char* name,
    uint32_t stack_size,
    FuriThreadCallback callback,
    void* context) {
    FuriThread* thread = furi_thread_alloc();
    furi_thread_set_name(thread, name);
    furi_thread_set_stack_size(thread, stack_size);
    furi_thread_set_callback(thread, callback);
    furi_thread_set_context(thread, context);
    return thread;
}

void furi_thread_free(FuriThread* thread) {
    furi_assert(thread);

    // Ensure that use join before free
    furi_assert(thread->state == FuriThreadStateStopped);
    furi_assert(thread->task_handle == NULL);

    if(thread->name) free(thread->name);
    if(thread->appid) free(thread->appid);
    furi_string_free(thread->output.buffer);

    free(thread);
}

void furi_thread_set_name(FuriThread* thread, const char* name) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    if(thread->name) free(thread->name);
    thread->name = name ? strdup(name) : NULL;
}

void furi_thread_set_appid(FuriThread* thread, const char* appid) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    if(thread->appid) free(thread->appid);
    thread->appid = appid ? strdup(appid) : NULL;
}

void furi_thread_mark_as_service(FuriThread* thread) {
    thread->is_service = true;
}

void furi_thread_set_stack_size(FuriThread* thread, size_t stack_size) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    furi_assert(stack_size % 4 == 0);
    thread->stack_size = stack_size;
}

void furi_thread_set_callback(FuriThread* thread, FuriThreadCallback callback) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    thread->callback = callback;
}

void furi_thread_set_context(FuriThread* thread, void* context) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    thread->context = context;
}

void furi_thread_set_priority(FuriThread* thread, FuriThreadPriority priority) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    furi_assert(priority >= FuriThreadPriorityIdle && priority <= FuriThreadPriorityIsr);
    thread->priority = priority;
}

void furi_thread_set_current_priority(FuriThreadPriority priority) {
    UBaseType_t new_priority = priority ? priority : FuriThreadPriorityNormal;
    vTaskPrioritySet(NULL, new_priority);
}

FuriThreadPriority furi_thread_get_current_priority() {
    return (FuriThreadPriority)uxTaskPriorityGet(NULL);
}

void furi_thread_set_state_callback(FuriThread* thread, FuriThreadStateCallback callback) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    thread->state_callback = callback;
}

void furi_thread_set_state_context(FuriThread* thread, void* context) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    thread->state_context = context;
}

FuriThreadState furi_thread_get_state(FuriThread* thread) {
    furi_assert(thread);
    return thread->state;
}

void furi_thread_start(FuriThread* thread) {
    furi_assert(thread);
    furi_assert(thread->callback);
    furi_assert(thread->state == FuriThreadStateStopped);
    furi_assert(thread->stack_size > 0 && thread->stack_size < (UINT16_MAX * sizeof(StackType_t)));

    furi_thread_set_state(thread, FuriThreadStateStarting);

    uint32_t stack = thread->stack_size / sizeof(StackType_t);
    UBaseType_t priority = thread->priority ? thread->priority : FuriThreadPriorityNormal;
    if(thread->is_service) {
        thread->task_handle = xTaskCreateStatic(
            furi_thread_body,
            thread->name,
            stack,
            thread,
            priority,
            memmgr_alloc_from_pool(sizeof(StackType_t) * stack),
            memmgr_alloc_from_pool(sizeof(StaticTask_t)));
    } else {
        BaseType_t ret = xTaskCreate(
            furi_thread_body, thread->name, stack, thread, priority, &thread->task_handle);
        furi_check(ret == pdPASS);
    }

    furi_check(thread->task_handle);
}

void furi_thread_cleanup_tcb_event(TaskHandle_t task) {
    FuriThread* thread = pvTaskGetThreadLocalStoragePointer(task, 0);
    if(thread) {
        // clear thread local storage
        vTaskSetThreadLocalStoragePointer(task, 0, NULL);
        furi_assert(thread->task_handle == task);
        thread->task_handle = NULL;
    }
}

bool furi_thread_join(FuriThread* thread) {
    furi_assert(thread);

    furi_check(furi_thread_get_current() != thread);

    // !!! IMPORTANT NOTICE !!!
    //
    // If your thread exited, but your app stuck here: some other thread uses
    // all cpu time, which delays kernel from releasing task handle
    while(thread->task_handle) {
        furi_delay_ms(10);
    }

    return true;
}

FuriThreadId furi_thread_get_id(FuriThread* thread) {
    furi_assert(thread);
    return thread->task_handle;
}

void furi_thread_enable_heap_trace(FuriThread* thread) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    thread->heap_trace_enabled = true;
}

void furi_thread_disable_heap_trace(FuriThread* thread) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    thread->heap_trace_enabled = false;
}

size_t furi_thread_get_heap_size(FuriThread* thread) {
    furi_assert(thread);
    furi_assert(thread->heap_trace_enabled == true);
    return thread->heap_size;
}

int32_t furi_thread_get_return_code(FuriThread* thread) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    return thread->ret;
}

FuriThreadId furi_thread_get_current_id() {
    return xTaskGetCurrentTaskHandle();
}

FuriThread* furi_thread_get_current() {
    FuriThread* thread = pvTaskGetThreadLocalStoragePointer(NULL, 0);
    return thread;
}

void furi_thread_yield() {
    furi_assert(!FURI_IS_IRQ_MODE());
    taskYIELD();
}

/* Limits */
#define MAX_BITS_TASK_NOTIFY 31U
#define MAX_BITS_EVENT_GROUPS 24U

#define THREAD_FLAGS_INVALID_BITS (~((1UL << MAX_BITS_TASK_NOTIFY) - 1U))
#define EVENT_FLAGS_INVALID_BITS (~((1UL << MAX_BITS_EVENT_GROUPS) - 1U))

uint32_t furi_thread_flags_set(FuriThreadId thread_id, uint32_t flags) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    uint32_t rflags;
    BaseType_t yield;

    if((hTask == NULL) || ((flags & THREAD_FLAGS_INVALID_BITS) != 0U)) {
        rflags = (uint32_t)FuriStatusErrorParameter;
    } else {
        rflags = (uint32_t)FuriStatusError;

        if(FURI_IS_IRQ_MODE()) {
            yield = pdFALSE;

            (void)xTaskNotifyIndexedFromISR(hTask, THREAD_NOTIFY_INDEX, flags, eSetBits, &yield);
            (void)xTaskNotifyAndQueryIndexedFromISR(
                hTask, THREAD_NOTIFY_INDEX, 0, eNoAction, &rflags, NULL);

            portYIELD_FROM_ISR(yield);
        } else {
            (void)xTaskNotifyIndexed(hTask, THREAD_NOTIFY_INDEX, flags, eSetBits);
            (void)xTaskNotifyAndQueryIndexed(hTask, THREAD_NOTIFY_INDEX, 0, eNoAction, &rflags);
        }
    }
    /* Return flags after setting */
    return (rflags);
}

uint32_t furi_thread_flags_clear(uint32_t flags) {
    TaskHandle_t hTask;
    uint32_t rflags, cflags;

    if(FURI_IS_IRQ_MODE()) {
        rflags = (uint32_t)FuriStatusErrorISR;
    } else if((flags & THREAD_FLAGS_INVALID_BITS) != 0U) {
        rflags = (uint32_t)FuriStatusErrorParameter;
    } else {
        hTask = xTaskGetCurrentTaskHandle();

        if(xTaskNotifyAndQueryIndexed(hTask, THREAD_NOTIFY_INDEX, 0, eNoAction, &cflags) ==
           pdPASS) {
            rflags = cflags;
            cflags &= ~flags;

            if(xTaskNotifyIndexed(hTask, THREAD_NOTIFY_INDEX, cflags, eSetValueWithOverwrite) !=
               pdPASS) {
                rflags = (uint32_t)FuriStatusError;
            }
        } else {
            rflags = (uint32_t)FuriStatusError;
        }
    }

    /* Return flags before clearing */
    return (rflags);
}

uint32_t furi_thread_flags_get(void) {
    TaskHandle_t hTask;
    uint32_t rflags;

    if(FURI_IS_IRQ_MODE()) {
        rflags = (uint32_t)FuriStatusErrorISR;
    } else {
        hTask = xTaskGetCurrentTaskHandle();

        if(xTaskNotifyAndQueryIndexed(hTask, THREAD_NOTIFY_INDEX, 0, eNoAction, &rflags) !=
           pdPASS) {
            rflags = (uint32_t)FuriStatusError;
        }
    }

    return (rflags);
}

uint32_t furi_thread_flags_wait(uint32_t flags, uint32_t options, uint32_t timeout) {
    uint32_t rflags, nval;
    uint32_t clear;
    TickType_t t0, td, tout;
    BaseType_t rval;

    if(FURI_IS_IRQ_MODE()) {
        rflags = (uint32_t)FuriStatusErrorISR;
    } else if((flags & THREAD_FLAGS_INVALID_BITS) != 0U) {
        rflags = (uint32_t)FuriStatusErrorParameter;
    } else {
        if((options & FuriFlagNoClear) == FuriFlagNoClear) {
            clear = 0U;
        } else {
            clear = flags;
        }

        rflags = 0U;
        tout = timeout;

        t0 = xTaskGetTickCount();
        do {
            rval = xTaskNotifyWaitIndexed(THREAD_NOTIFY_INDEX, 0, clear, &nval, tout);

            if(rval == pdPASS) {
                rflags &= flags;
                rflags |= nval;

                if((options & FuriFlagWaitAll) == FuriFlagWaitAll) {
                    if((flags & rflags) == flags) {
                        break;
                    } else {
                        if(timeout == 0U) {
                            rflags = (uint32_t)FuriStatusErrorResource;
                            break;
                        }
                    }
                } else {
                    if((flags & rflags) != 0) {
                        break;
                    } else {
                        if(timeout == 0U) {
                            rflags = (uint32_t)FuriStatusErrorResource;
                            break;
                        }
                    }
                }

                /* Update timeout */
                td = xTaskGetTickCount() - t0;

                if(td > tout) {
                    tout = 0;
                } else {
                    tout -= td;
                }
            } else {
                if(timeout == 0) {
                    rflags = (uint32_t)FuriStatusErrorResource;
                } else {
                    rflags = (uint32_t)FuriStatusErrorTimeout;
                }
            }
        } while(rval != pdFAIL);
    }

    /* Return flags before clearing */
    return (rflags);
}

uint32_t furi_thread_enumerate(FuriThreadId* thread_array, uint32_t array_items) {
    uint32_t i, count;
    TaskStatus_t* task;

    if(FURI_IS_IRQ_MODE() || (thread_array == NULL) || (array_items == 0U)) {
        count = 0U;
    } else {
        vTaskSuspendAll();

        count = uxTaskGetNumberOfTasks();
        task = pvPortMalloc(count * sizeof(TaskStatus_t));

        if(task != NULL) {
            count = uxTaskGetSystemState(task, count, NULL);

            for(i = 0U; (i < count) && (i < array_items); i++) {
                thread_array[i] = (FuriThreadId)task[i].xHandle;
            }
            count = i;
        }
        (void)xTaskResumeAll();

        vPortFree(task);
    }

    return (count);
}

const char* furi_thread_get_name(FuriThreadId thread_id) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    const char* name;

    if(FURI_IS_IRQ_MODE() || (hTask == NULL)) {
        name = NULL;
    } else {
        name = pcTaskGetName(hTask);
    }

    return (name);
}

const char* furi_thread_get_appid(FuriThreadId thread_id) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    const char* appid = "system";

    if(!FURI_IS_IRQ_MODE() && (hTask != NULL)) {
        FuriThread* thread = (FuriThread*)pvTaskGetThreadLocalStoragePointer(hTask, 0);
        if(thread) {
            appid = thread->appid;
        }
    }

    return (appid);
}

uint32_t furi_thread_get_stack_space(FuriThreadId thread_id) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    uint32_t sz;

    if(FURI_IS_IRQ_MODE() || (hTask == NULL)) {
        sz = 0U;
    } else {
        sz = (uint32_t)(uxTaskGetStackHighWaterMark(hTask) * sizeof(StackType_t));
    }

    return (sz);
}

static size_t __furi_thread_stdout_write(FuriThread* thread, const char* data, size_t size) {
    if(thread->output.write_callback != NULL) {
        thread->output.write_callback(data, size);
    } else {
        furi_hal_console_tx((const uint8_t*)data, size);
    }
    return size;
}

static int32_t __furi_thread_stdout_flush(FuriThread* thread) {
    FuriString* buffer = thread->output.buffer;
    size_t size = furi_string_size(buffer);
    if(size > 0) {
        __furi_thread_stdout_write(thread, furi_string_get_cstr(buffer), size);
        furi_string_reset(buffer);
    }
    return 0;
}

void furi_thread_set_stdout_callback(FuriThreadStdoutWriteCallback callback) {
    FuriThread* thread = furi_thread_get_current();
    furi_assert(thread);
    __furi_thread_stdout_flush(thread);
    thread->output.write_callback = callback;
}

FuriThreadStdoutWriteCallback furi_thread_get_stdout_callback() {
    FuriThread* thread = furi_thread_get_current();
    furi_assert(thread);
    return thread->output.write_callback;
}

size_t furi_thread_stdout_write(const char* data, size_t size) {
    FuriThread* thread = furi_thread_get_current();
    furi_assert(thread);
    if(size == 0 || data == NULL) {
        return __furi_thread_stdout_flush(thread);
    } else {
        if(data[size - 1] == '\n') {
            // if the last character is a newline, we can flush buffer and write data as is, wo buffers
            __furi_thread_stdout_flush(thread);
            __furi_thread_stdout_write(thread, data, size);
        } else {
            // string_cat doesn't work here because we need to write the exact size data
            for(size_t i = 0; i < size; i++) {
                furi_string_push_back(thread->output.buffer, data[i]);
                if(data[i] == '\n') {
                    __furi_thread_stdout_flush(thread);
                }
            }
        }
    }

    return size;
}

int32_t furi_thread_stdout_flush() {
    FuriThread* thread = furi_thread_get_current();
    furi_assert(thread);
    return __furi_thread_stdout_flush(thread);
}

void furi_thread_suspend(FuriThreadId thread_id) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    vTaskSuspend(hTask);
}

void furi_thread_resume(FuriThreadId thread_id) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    if(FURI_IS_IRQ_MODE()) {
        xTaskResumeFromISR(hTask);
    } else {
        vTaskResume(hTask);
    }
}

bool furi_thread_is_suspended(FuriThreadId thread_id) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    return eTaskGetState(hTask) == eSuspended;
}
