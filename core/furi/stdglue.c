#include "stdglue.h"
#include <main.h>

#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef DEBUG_UART;

static ssize_t stdout_write(void* _cookie, const char* data, size_t size) {
    if(data == 0) {
        /*
         * This means that we should flush internal buffers.  Since we
         * don't we just return.  (Remember, "handle" == -1 means that all
         * handles should be flushed.)
         */
        return 0;
    }

    HAL_UART_Transmit(&DEBUG_UART, (uint8_t*)data, (uint16_t)size, HAL_MAX_DELAY);

    return size;
}

bool furi_stdglue_init() {
    FILE* fp = fopencookie(
        NULL,
        "w",
        (cookie_io_functions_t){
            .read = NULL,
            .write = stdout_write,
            .seek = NULL,
            .close = NULL,
        });
    setvbuf(fp, NULL, _IONBF, 0);
    stdout = fp;

    return true;
}
