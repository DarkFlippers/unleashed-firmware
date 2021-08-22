#include "check.h"
#include "furi-hal-task.h"
#include <furi-hal-console.h>
#include <stdio.h>

void __furi_abort(void);

void __furi_print_name(void) {
    furi_hal_console_puts("\r\n\033[0;31m[E]");
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
    furi_hal_console_puts("\033[0m");
}

void __furi_check(void) {
    __furi_print_name();
    furi_hal_console_puts("assertion failed\r\n");
    __furi_abort();
}

void __furi_abort(void) {
    __disable_irq();
    asm("bkpt 1");
    while(1) {
    }
}