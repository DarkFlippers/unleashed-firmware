#include "log.h"
#include "check.h"
#include "mutex.h"
#include <furi_hal.h>

#define FURI_LOG_LEVEL_DEFAULT FuriLogLevelInfo

typedef struct {
    FuriLogLevel log_level;
    FuriLogPuts puts;
    FuriLogTimestamp timetamp;
    FuriMutex* mutex;
} FuriLogParams;

static FuriLogParams furi_log;

void furi_log_init() {
    // Set default logging parameters
    furi_log.log_level = FURI_LOG_LEVEL_DEFAULT;
    furi_log.puts = furi_hal_console_puts;
    furi_log.timetamp = furi_get_tick;
    furi_log.mutex = furi_mutex_alloc(FuriMutexTypeNormal);
}

void furi_log_print_format(FuriLogLevel level, const char* tag, const char* format, ...) {
    if(level <= furi_log.log_level &&
       furi_mutex_acquire(furi_log.mutex, FuriWaitForever) == FuriStatusOk) {
        string_t string;
        string_init(string);

        const char* color = FURI_LOG_CLR_RESET;
        const char* log_letter = " ";
        switch(level) {
        case FuriLogLevelError:
            color = FURI_LOG_CLR_E;
            log_letter = "E";
            break;
        case FuriLogLevelWarn:
            color = FURI_LOG_CLR_W;
            log_letter = "W";
            break;
        case FuriLogLevelInfo:
            color = FURI_LOG_CLR_I;
            log_letter = "I";
            break;
        case FuriLogLevelDebug:
            color = FURI_LOG_CLR_D;
            log_letter = "D";
            break;
        case FuriLogLevelTrace:
            color = FURI_LOG_CLR_T;
            log_letter = "T";
            break;
        default:
            break;
        }

        // Timestamp
        string_printf(
            string,
            "%lu %s[%s][%s] " FURI_LOG_CLR_RESET,
            furi_log.timetamp(),
            color,
            log_letter,
            tag);
        furi_log.puts(string_get_cstr(string));
        string_reset(string);

        va_list args;
        va_start(args, format);
        string_vprintf(string, format, args);
        va_end(args);

        furi_log.puts(string_get_cstr(string));
        string_clear(string);

        furi_log.puts("\r\n");

        furi_mutex_release(furi_log.mutex);
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
