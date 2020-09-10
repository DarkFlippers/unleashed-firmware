#define _GNU_SOURCE
#include <stdio.h>
#include "furi.h"
#include "main.h"

extern UART_HandleTypeDef DEBUG_UART;

void handle_uart_write(const void* data, size_t size, void* ctx) {
	HAL_UART_Transmit(&DEBUG_UART, (uint8_t*)data, (uint16_t)size, HAL_MAX_DELAY);
}

static ssize_t stdout_write(void *_cookie, const char *buf, size_t n) {
    FuriRecordSubscriber *log = pvTaskGetThreadLocalStoragePointer(NULL, 0);
    if (log == NULL) {
        log = furi_open("tty", false, false, NULL, NULL, NULL);
        if (log == NULL) {
            return -1;
        }
        vTaskSetThreadLocalStoragePointer(NULL, 0, log);
    }
    if (buf == 0) {
        /*
         * This means that we should flush internal buffers.  Since we
         * don't we just return.  (Remember, "handle" == -1 means that all
         * handles should be flushed.)
         */
        return 0;
    }

    furi_write(log, buf, n);

    return n;
}

bool register_tty_uart() {
	if(!furi_create("tty", NULL, 0)) {
		return false;
	}

	if(furi_open("tty", false, false, handle_uart_write, NULL, NULL) == NULL) {
		return false;
	}

    FILE* fp = fopencookie(NULL, "w", (cookie_io_functions_t) {
        .read = NULL,
        .write = stdout_write,
        .seek = NULL,
        .close = NULL,
    });
    setvbuf(fp, NULL, _IONBF, 0);
    stdout = fp;

	return true;
}
