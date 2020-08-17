#define _GNU_SOURCE
#include "main.h"
#include <stdio.h>

extern UART_HandleTypeDef DEBUG_UART;

ssize_t uart_write(void* cookie, const char * buffer, size_t size) {
    if (buffer == 0) {
        /*
         * This means that we should flush internal buffers.  Since we
         * don't we just return.  (Remember, "handle" == -1 means that all
         * handles should be flushed.)
         */
        return 0;
    }
    
    return (ssize_t)HAL_UART_Transmit(&DEBUG_UART, (uint8_t*)buffer, (uint16_t)size, HAL_MAX_DELAY);
}

FILE* get_debug() {
    FILE* fp = fopencookie(NULL,"w+", (cookie_io_functions_t){
        .read  = NULL,
        .write = uart_write,
        .seek  = NULL,
        .close = NULL
    });

    setvbuf(fp, NULL, _IONBF, 0);

    return fp;
}