#include "flipper.h"
#include <applications.h>
#include <furi.h>
#include <furi-hal-version.h>

static void flipper_print_version(const char* target, const Version* version) {
    if(version) {
        FURI_LOG_I(
            "FLIPPER",
            "\r\n\t%s version:\t%s\r\n"
            "\tBuild date:\t\t%s\r\n"
            "\tGit Commit:\t\t%s (%s)\r\n"
            "\tGit Branch:\t\t%s",
            target,
            version_get_version(version),
            version_get_builddate(version),
            version_get_githash(version),
            version_get_gitbranchnum(version),
            version_get_gitbranch(version));
    } else {
        FURI_LOG_I("FLIPPER", "No build info for %s", target);
    }
}

void flipper_init() {
    const Version* version;

    version = (const Version*)furi_hal_version_get_boot_version();
    flipper_print_version("Bootloader", version);

    version = (const Version*)furi_hal_version_get_firmware_version();
    flipper_print_version("Firmware", version);

    FURI_LOG_I("FLIPPER", "starting services");

    for(size_t i = 0; i < FLIPPER_SERVICES_COUNT; i++) {
        FURI_LOG_I("FLIPPER", "starting service %s", FLIPPER_SERVICES[i].name);

        FuriThread* thread = furi_thread_alloc();

        furi_thread_set_name(thread, FLIPPER_SERVICES[i].name);
        furi_thread_set_stack_size(thread, FLIPPER_SERVICES[i].stack_size);
        furi_thread_set_callback(thread, FLIPPER_SERVICES[i].app);

        furi_thread_start(thread);
    }

    FURI_LOG_I("FLIPPER", "services startup complete");
}
