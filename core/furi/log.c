#include "log.h"
#include <stm32wbxx_hal.h>
#include "check.h"
#include <cmsis_os2.h>

typedef struct {
    FuriLogLevel log_level;
    FuriLogPrint print;
    FuriLogVPrint vprint;
    FuriLogTimestamp timetamp;
    osMutexId_t mutex;
} FuriLogParams;

static FuriLogParams furi_log;

void furi_log_init() {
    // Set default logging parameters
    furi_log.log_level = FURI_LOG_LEVEL;
    furi_log.print = printf;
    furi_log.vprint = vprintf;
    furi_log.timetamp = HAL_GetTick;
    furi_log.mutex = osMutexNew(NULL);
}

void furi_log_print(FuriLogLevel level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    if(level <= furi_log.log_level) {
        osMutexAcquire(furi_log.mutex, osWaitForever);
        furi_log.print("%lu ", furi_log.timetamp());
        furi_log.vprint(format, args);
        osMutexRelease(furi_log.mutex);
    }
    va_end(args);
}

void furi_log_set_level(FuriLogLevel level) {
    furi_log.log_level = level;
}

FuriLogLevel furi_log_get_level(void) {
    return furi_log.log_level;
}

void furi_log_set_print(FuriLogPrint print, FuriLogVPrint vprint) {
    furi_assert(print);
    furi_assert(vprint);

    furi_log.print = print;
    furi_log.vprint = vprint;
}

void furi_log_set_timestamp(FuriLogTimestamp timestamp) {
    furi_assert(timestamp);

    furi_log.timetamp = timestamp;
}
