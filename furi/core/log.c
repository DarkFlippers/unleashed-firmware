#include "log.h"
#include "check.h"
#include "mutex.h"
#include <furi_hal.h>

#define FURI_LOG_LEVEL_DEFAULT FuriLogLevelInfo

typedef struct {
    FuriLogLevel log_level;
    FuriLogPuts puts;
    FuriLogTimestamp timestamp;
    FuriMutex* mutex;
} FuriLogParams;

static FuriLogParams furi_log;

void furi_log_init() {
    // Set default logging parameters
    furi_log.log_level = FURI_LOG_LEVEL_DEFAULT;
    furi_log.puts = furi_hal_console_puts;
    furi_log.timestamp = furi_get_tick;
    furi_log.mutex = furi_mutex_alloc(FuriMutexTypeNormal);
}

void furi_log_print_format(FuriLogLevel level, const char* tag, const char* format, ...) {
    if(level <= furi_log.log_level &&
       furi_mutex_acquire(furi_log.mutex, FuriWaitForever) == FuriStatusOk) {
        FuriString* string;
        string = furi_string_alloc();

        const char* color = _FURI_LOG_CLR_RESET;
        const char* log_letter = " ";
        switch(level) {
        case FuriLogLevelError:
            color = _FURI_LOG_CLR_E;
            log_letter = "E";
            break;
        case FuriLogLevelWarn:
            color = _FURI_LOG_CLR_W;
            log_letter = "W";
            break;
        case FuriLogLevelInfo:
            color = _FURI_LOG_CLR_I;
            log_letter = "I";
            break;
        case FuriLogLevelDebug:
            color = _FURI_LOG_CLR_D;
            log_letter = "D";
            break;
        case FuriLogLevelTrace:
            color = _FURI_LOG_CLR_T;
            log_letter = "T";
            break;
        default:
            break;
        }

        // Timestamp
        furi_string_printf(
            string,
            "%lu %s[%s][%s] " _FURI_LOG_CLR_RESET,
            furi_log.timestamp(),
            color,
            log_letter,
            tag);
        furi_log.puts(furi_string_get_cstr(string));
        furi_string_reset(string);

        va_list args;
        va_start(args, format);
        furi_string_vprintf(string, format, args);
        va_end(args);

        furi_log.puts(furi_string_get_cstr(string));
        furi_string_free(string);

        furi_log.puts("\r\n");

        furi_mutex_release(furi_log.mutex);
    }
}

void furi_log_print_raw_format(FuriLogLevel level, const char* format, ...) {
    if(level <= furi_log.log_level &&
       furi_mutex_acquire(furi_log.mutex, FuriWaitForever) == FuriStatusOk) {
        FuriString* string;
        string = furi_string_alloc();
        va_list args;
        va_start(args, format);
        furi_string_vprintf(string, format, args);
        va_end(args);

        furi_log.puts(furi_string_get_cstr(string));
        furi_string_free(string);

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
    furi_log.timestamp = timestamp;
}
