#include "furi.h"
#include <applications.h>

// for testing purpose
uint32_t exitcode = 0;

void set_exitcode(uint32_t _exitcode) {
    exitcode = _exitcode;
}

void furi_init() {
    gpio_api_init();
    api_interrupt_init();
    furi_record_init();
    furi_stdglue_init();
}

int systemd() {
    furi_init();
    printf("[systemd] furi initialized\r\n");
    // FURI startup
    for(size_t i = 0; i < FLIPPER_SERVICES_size(); i++) {
        printf("[systemd] starting service %s\r\n", FLIPPER_SERVICES[i].name);
        osThreadAttr_t* attr = furi_alloc(sizeof(osThreadAttr_t));
        attr->name = FLIPPER_SERVICES[i].name;
        attr->stack_size = FLIPPER_SERVICES[i].stack_size;
        osThreadNew(FLIPPER_SERVICES[i].app, NULL, attr);
    }
    printf("[systemd] all services started\r\n");

    while(1) {
        osThreadSuspend(osThreadGetId());
    }

    printf("[systemd] === Bye from Flipper Zero! ===\r\n");

    return (int)exitcode;
}