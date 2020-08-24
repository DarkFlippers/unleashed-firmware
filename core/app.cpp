#include "flipper.h"
#include <stdio.h>

extern "C" {
    #include "startup.h"
    #include "furi.h"
    #include "debug.h"
}

extern "C" void app() {
    // FURI startup
    FuriApp* handlers[sizeof(FLIPPER_STARTUP)/sizeof(FLIPPER_STARTUP[0])];

    for(size_t i = 0; i < sizeof(FLIPPER_STARTUP)/sizeof(FLIPPER_STARTUP[0]); i++) {
        handlers[i] = furiac_start(FLIPPER_STARTUP[i].app, FLIPPER_STARTUP[i].name, NULL);
    }

    bool is_alive = false;
    do {
        is_alive = false;
        for(size_t i = 0; i < sizeof(FLIPPER_STARTUP)/sizeof(FLIPPER_STARTUP[0]); i++) {
            if(handlers[i]->handler != NULL) {
                is_alive = true;
            }
        }
        delay(500);
        // TODO add deferred event queue here
    } while(is_alive);
}