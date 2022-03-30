#include "log.h"
#include "check.h"
#include <cmsis_os2.h>
#include <furi_hal.h>

#define FURI_LOG_LEVEL_DEFAULT FuriLogLevelInfo

typedef struct {
    FuriLogLevel log_level;
    FuriLogPuts puts;
    FuriLogTimestamp timetamp;
    osMutexId_t mutex;
} FuriLogParams;

static FuriLogParams furi_log;

void furi_log_init() {
    // Set default logging parameters
    furi_log.log_level = FURI_LOG_LEVEL_DEFAULT;
    furi_log.puts = furi_hal_console_puts;
    furi_log.timetamp = furi_hal_get_tick;
    furi_log.mutex = osMutexNew(NULL);
}

void furi_log_print(FuriLogLevel level, const char* format, ...) {
    if(level <= furi_log.log_level && osMutexAcquire(furi_log.mutex, osWaitForever) == osOK) {
        string_t string;

        // Timestamp
        string_init_printf(string, "%lu ", furi_log.timetamp());
        furi_log.puts(string_get_cstr(string));
        string_clear(string);

        va_list args;
        va_start(args, format);
        string_init_vprintf(string, format, args);
        va_end(args);

        furi_log.puts(string_get_cstr(string));
        string_clear(string);

        osMutexRelease(furi_log.mutex);
    }
}

void furi_log_set_level(FuriLogLevel level) {
    if(level == FuriLogLevelDefault) {
        level = FURI_LOG_LEVEL_DEFAULT;
    }
    furi_log.log_level = level;
}

FuriLogLevel furi_log_get_level(void) {
    return furi_log.log_level;
}

void furi_log_set_puts(FuriLogPuts puts) {
    furi_assert(puts);
    furi_log.puts = puts;
}

void furi_log_set_timestamp(FuriLogTimestamp timestamp) {
    furi_assert(timestamp);
    furi_log.timetamp = timestamp;
}
