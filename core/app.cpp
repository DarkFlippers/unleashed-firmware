#include <stdio.h>
#include "flipper.h"
#include "flipper_v2.h"

extern "C" {
#include "log.h"
#include "applications.h"
#include "tty_uart.h"
}

// for testing purpose
uint32_t exitcode = 0;
extern "C" void set_exitcode(uint32_t _exitcode) {
    exitcode = _exitcode;
}

extern "C" int app() {
    init_flipper_api();
    register_tty_uart();

    FuriRecordSubscriber* log = get_default_log();
    fuprintf(log, "\n=== Welcome to Flipper Zero! ===\n\n");

    // FURI startup
    const size_t flipper_app_count = sizeof(FLIPPER_STARTUP) / sizeof(FLIPPER_STARTUP[0]);
    FuriApp* handlers[flipper_app_count];

    for(size_t i = 0; i < flipper_app_count; i++) {
        // TODO create a dependency tree and run tasks in the desired order
        furiac_wait_libs(&FLIPPER_STARTUP[i].libs);
        handlers[i] = furiac_start(FLIPPER_STARTUP[i].app, FLIPPER_STARTUP[i].name, NULL);
    }

    bool is_alive = false;
    do {
        is_alive = false;
        for(size_t i = 0; i < flipper_app_count; i++) {
            if(handlers[i]->handler != NULL) {
                is_alive = true;
            }
        }
        delay(500);
    } while(is_alive);

    fuprintf(log, "\n=== Bye from Flipper Zero! ===\n\n");

    return (int)exitcode;
}