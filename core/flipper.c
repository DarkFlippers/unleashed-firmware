#include "flipper.h"
#include <applications.h>
#include <furi.h>

void flipper_init() {
    printf("[flipper] Build date:" BUILD_DATE ". "
           "Git Commit:" GIT_COMMIT ". "
           "Git Branch:" GIT_BRANCH ". "
           "Commit Number:" GIT_BRANCH_NUM "\r\n");

    printf("[flipper] starting services\r\n");

    for(size_t i = 0; i < FLIPPER_SERVICES_COUNT; i++) {
        printf("[flipper] starting service %s\r\n", FLIPPER_SERVICES[i].name);

        FuriThread* thread = furi_thread_alloc();

        furi_thread_set_name(thread, FLIPPER_SERVICES[i].name);
        furi_thread_set_stack_size(thread, FLIPPER_SERVICES[i].stack_size);
        furi_thread_set_callback(thread, FLIPPER_SERVICES[i].app);

        furi_thread_start(thread);
    }

    printf("[flipper] services startup complete\r\n");
}