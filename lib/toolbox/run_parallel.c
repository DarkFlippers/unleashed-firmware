#include "run_parallel.h"

#include <stdlib.h>

static void run_parallel_thread_state(FuriThread* thread, FuriThreadState state, void* context) {
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
    }
}

void run_parallel(FuriThreadCallback callback, void* context, uint32_t stack_size) {
    FuriThread* thread = furi_thread_alloc_ex(NULL, stack_size, callback, context);
    furi_thread_set_state_callback(thread, run_parallel_thread_state);
    furi_thread_start(thread);
}
