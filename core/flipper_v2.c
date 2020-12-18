#include "flipper_v2.h"

bool init_flipper_api(void) {
    bool no_errors = true;

    if(!furi_init()) {
        no_errors = false;
    }

    if(!gpio_api_init()) {
        no_errors = false;
    }

    if(!api_interrupt_init()) {
        no_errors = false;
    }

    return no_errors;
}