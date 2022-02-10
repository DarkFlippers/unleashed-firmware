#include "check.h"
#include "furi_hal_task.h"
#include <furi_hal_console.h>
#include <furi_hal_rtc.h>
#include <stdio.h>

static void __furi_print_name() {
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

static void __furi_halt() {
    asm volatile("loop:      \n"
                 "bkpt 0x00  \n"
                 "wfi        \n"
                 "b loop     \n"
                 :
                 :
                 : "memory");
}

void furi_crash(const char* message) {
    __disable_irq();

    if(message == NULL) {
        message = "Fatal Error";
    }

    furi_hal_console_puts("\r\n\033[0;31m[CRASH]");
    __furi_print_name();
    furi_hal_console_puts(message);
#ifdef FURI_DEBUG
    furi_hal_console_puts("\r\nSystem halted. Connect debugger for more info\r\n");
    furi_hal_console_puts("\033[0m\r\n");
    __furi_halt();
#else
    furi_hal_rtc_set_fault_data((uint32_t)message);
    furi_hal_console_puts("\r\nRebooting system.\r\n");
    furi_hal_console_puts("\033[0m\r\n");
    NVIC_SystemReset();
#endif
}

void furi_halt(const char* message) {
    __disable_irq();

    if(message == NULL) {
        message = "System halt requested.";
    }

    furi_hal_console_puts("\r\n\033[0;31m[HALT]");
    __furi_print_name();
    furi_hal_console_puts(message);
    furi_hal_console_puts("\r\nSystem halted. Bye-bye!\r\n");
    furi_hal_console_puts("\033[0m\r\n");
    __furi_halt();
}
