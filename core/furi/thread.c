#include "thread.h"
#include "memmgr.h"
#include "memmgr_heap.h"
#include "check.h"

#include <m-string.h>

struct FuriThread {
    FuriThreadState state;
    int32_t ret;

    FuriThreadCallback callback;
    void* context;

    FuriThreadStateCallback state_callback;
    void* state_context;

    osThreadAttr_t attr;
    volatile osThreadId_t id;

    bool heap_trace_enabled;
    size_t heap_size;
};

void furi_thread_set_state(FuriThread* thread, FuriThreadState state) {
    furi_assert(thread);
    thread->state = state;
    if(thread->state_callback) {
        thread->state_callback(state, thread->state_context);
    }
}

static void furi_thread_body(void* context) {
    furi_assert(context);
    FuriThread* thread = context;

    furi_assert(thread->state == FuriThreadStateStarting);
    furi_thread_set_state(thread, FuriThreadStateRunning);

    osThreadId_t thread_id = osThreadGetId();
    if(thread->heap_trace_enabled == true) {
        memmgr_heap_enable_thread_trace(thread_id);
    }

    thread->ret = thread->callback(thread->context);

    if(thread->heap_trace_enabled == true) {
        thread->heap_size = memmgr_heap_get_thread_memory(thread_id);
        memmgr_heap_disable_thread_trace(thread_id);
    }

    furi_assert(thread->state == FuriThreadStateRunning);
    furi_thread_set_state(thread, FuriThreadStateStopped);

    osThreadExit();
}

FuriThread* furi_thread_alloc() {
    FuriThread* thread = malloc(sizeof(FuriThread));

    return thread;
}

void furi_thread_free(FuriThread* thread) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);

    if(thread->attr.name) free((void*)thread->attr.name);
    free(thread);
}

void furi_thread_set_name(FuriThread* thread, const char* name) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    if(thread->attr.name) free((void*)thread->attr.name);
    thread->attr.name = strdup(name);
}

void furi_thread_set_stack_size(FuriThread* thread, size_t stack_size) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    thread->attr.stack_size = stack_size;
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

bool furi_thread_start(FuriThread* thread) {
    furi_assert(thread);
    furi_assert(thread->callback);
    furi_assert(thread->state == FuriThreadStateStopped);
    furi_assert(thread->attr.stack_size > 0);
    furi_thread_set_state(thread, FuriThreadStateStarting);
    thread->id = osThreadNew(furi_thread_body, thread, &thread->attr);
    if(thread->id) {
        return true;
    } else {
        furi_assert(thread->state == FuriThreadStateStarting);
        furi_thread_set_state(thread, FuriThreadStateStopped);
        return false;
    }
}

osStatus_t furi_thread_terminate(FuriThread* thread) {
    furi_assert(thread);
    osStatus_t ret = osThreadTerminate(thread->id);
    if(ret == osOK) {
        furi_thread_set_state(thread, FuriThreadStateStopped);
    }
    return ret;
}

osStatus_t furi_thread_join(FuriThread* thread) {
    furi_assert(thread);
    while(thread->state != FuriThreadStateStopped) {
        osDelay(10);
    }
    return osOK;
}

osThreadId_t furi_thread_get_thread_id(FuriThread* thread) {
    return thread->id;
}

void furi_thread_enable_heap_trace(FuriThread* thread) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    furi_assert(thread->heap_trace_enabled == false);
    thread->heap_trace_enabled = true;
}

void furi_thread_disable_heap_trace(FuriThread* thread) {
    furi_assert(thread);
    furi_assert(thread->state == FuriThreadStateStopped);
    furi_assert(thread->heap_trace_enabled == true);
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
