#include "flipper.h"
#include <applications.h>
#include <furi.h>
#include <furi_hal_version.h>
#include <furi_hal_memory.h>

#define TAG "Flipper"

static void flipper_print_version(const char* target, const Version* version) {
    if(version) {
        FURI_LOG_I(
            TAG,
            "\r\n\t%s version:\t%s\r\n"
            "\tBuild date:\t\t%s\r\n"
            "\tGit Commit:\t\t%s (%s)%s\r\n"
            "\tGit Branch:\t\t%s",
            target,
            version_get_version(version),
            version_get_builddate(version),
            version_get_githash(version),
            version_get_gitbranchnum(version),
            version_get_dirty_flag(version) ? " (dirty)" : "",
            version_get_gitbranch(version));
    } else {
        FURI_LOG_I(TAG, "No build info for %s", target);
    }
}

void flipper_init() {
    flipper_print_version("Firmware", furi_hal_version_get_firmware_version());

    FURI_LOG_I(TAG, "starting services");

    for(size_t i = 0; i < FLIPPER_SERVICES_COUNT; i++) {
        FURI_LOG_I(TAG, "starting service %s", FLIPPER_SERVICES[i].name);

        FuriThread* thread = furi_thread_alloc();

        furi_thread_set_name(thread, FLIPPER_SERVICES[i].name);
        furi_thread_set_stack_size(thread, FLIPPER_SERVICES[i].stack_size);
        furi_thread_set_callback(thread, FLIPPER_SERVICES[i].app);
        furi_thread_mark_as_service(thread);

        furi_thread_start(thread);
    }

    FURI_LOG_I(TAG, "services startup complete");
}

void vApplicationGetIdleTaskMemory(
    StaticTask_t** tcb_ptr,
    StackType_t** stack_ptr,
    uint32_t* stack_size) {
    *tcb_ptr = memmgr_alloc_from_pool(sizeof(StaticTask_t));
    *stack_ptr = memmgr_alloc_from_pool(sizeof(StackType_t) * configMINIMAL_STACK_SIZE);
    *stack_size = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(
    StaticTask_t** tcb_ptr,
    StackType_t** stack_ptr,
    uint32_t* stack_size) {
    *tcb_ptr = memmgr_alloc_from_pool(sizeof(StaticTask_t));
    *stack_ptr = memmgr_alloc_from_pool(sizeof(StackType_t) * configTIMER_TASK_STACK_DEPTH);
    *stack_size = configTIMER_TASK_STACK_DEPTH;
}