#include "flipper.h"
#include <applications.h>
#include <furi.h>
#include <version.h>
#include <api-hal-version.h>

static void flipper_print_version(const Version* version) {
    if(version) {
        printf("\tVersion:\t%s\r\n", version_get_version(version));
        printf("\tBuild date:\t%s\r\n", version_get_builddate(version));
        printf(
            "\tGit Commit:\t%s (%s)\r\n",
            version_get_githash(version),
            version_get_gitbranchnum(version));
        printf("\tGit Branch:\t%s\r\n", version_get_gitbranch(version));
    } else {
        printf("\tNo build info\r\n");
    }
}

void flipper_init() {
    const Version* version;

    version = (const Version*)api_hal_version_get_boot_version();
    printf("Bootloader\r\n");
    flipper_print_version(version);

    version = (const Version*)api_hal_version_get_fw_version();
    printf("Firmware\r\n");
    flipper_print_version(version);

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
