#include "check.h"
#include "furi-hal-task.h"
#include <furi-hal-console.h>
#include <furi-hal-rtc.h>
#include <stdio.h>

__attribute__((always_inline)) inline static void __furi_print_name() {
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

__attribute__((always_inline)) inline static void __furi_halt() {
    asm volatile("bkpt 0x00  \n"
                 "loop:      \n"
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
