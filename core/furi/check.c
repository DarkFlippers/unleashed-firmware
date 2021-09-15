#include "check.h"
#include "furi-hal-task.h"
#include <furi-hal-console.h>
#include <stdio.h>

void __furi_print_name(void) {
    if(task_is_isr_context()) {
        furi_hal_console_puts("[ISR] ");
    } else {
        const char* name = osThreadGetName(osThreadGetId());
        if(name == NULL) {
            furi_hal_console_puts("[main] ");
        } else {
            furi_hal_console_puts("[");
            furi_hal_console_puts(name);
            furi_hal_console_puts("] ");
        }
    }
}

void __furi_abort(void) {
    __disable_irq();
    asm("bkpt 1");
    while(1) {
    }
}

void furi_crash(const char* message) {
    furi_hal_console_puts("\r\n\033[0;31m[CRASH]");
    __furi_print_name();
    furi_hal_console_puts(message ? message : "Programming Error");
    furi_hal_console_puts("\r\nSystem halted. Connect debugger for more info\r\n");
    furi_hal_console_puts("\033[0m\r\n");
    __furi_abort();
}
