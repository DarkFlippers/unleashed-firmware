/**
 * @file log.h
 * Furi Logging system
 */
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriLogLevelDefault = 0,
    FuriLogLevelNone = 1,
    FuriLogLevelError = 2,
    FuriLogLevelWarn = 3,
    FuriLogLevelInfo = 4,
    FuriLogLevelDebug = 5,
    FuriLogLevelTrace = 6,
} FuriLogLevel;

#define FURI_LOG_CLR(clr) "\033[0;" clr "m"
#define FURI_LOG_CLR_RESET "\033[0m"

#define FURI_LOG_CLR_BLACK "30"
#define FURI_LOG_CLR_RED "31"
#define FURI_LOG_CLR_GREEN "32"
#define FURI_LOG_CLR_BROWN "33"
#define FURI_LOG_CLR_BLUE "34"
#define FURI_LOG_CLR_PURPLE "35"

#define FURI_LOG_CLR_E FURI_LOG_CLR(FURI_LOG_CLR_RED)
#define FURI_LOG_CLR_W FURI_LOG_CLR(FURI_LOG_CLR_BROWN)
#define FURI_LOG_CLR_I FURI_LOG_CLR(FURI_LOG_CLR_GREEN)
#define FURI_LOG_CLR_D FURI_LOG_CLR(FURI_LOG_CLR_BLUE)
#define FURI_LOG_CLR_T FURI_LOG_CLR(FURI_LOG_CLR_PURPLE)

typedef void (*FuriLogPuts)(const char* data);
typedef uint32_t (*FuriLogTimestamp)(void);

/** Initialize logging */
void furi_log_init();

/** Print log record
 * 
 * @param level 
 * @param tag 
 * @param format 
 * @param ... 
 */
void furi_log_print_format(FuriLogLevel level, const char* tag, const char* format, ...);

/** Set log level
 *
 * @param[in]  level  The level
 */
void furi_log_set_level(FuriLogLevel level);

/** Get log level
 *
 * @return     The furi log level.
 */
FuriLogLevel furi_log_get_level();

/** Set log output callback
 *
 * @param[in]  puts  The puts callback
 */
void furi_log_set_puts(FuriLogPuts puts);

/** Set timestamp callback
 *
 * @param[in]  timestamp  The timestamp callback
 */
void furi_log_set_timestamp(FuriLogTimestamp timestamp);

/** Log methods
 *
 * @param      tag     The application tag
 * @param      format  The format
 * @param      ...     VA Args
 */
#define FURI_LOG_E(tag, format, ...) \
    furi_log_print_format(FuriLogLevelError, tag, format, ##__VA_ARGS__)
#define FURI_LOG_W(tag, format, ...) \
    furi_log_print_format(FuriLogLevelWarn, tag, format, ##__VA_ARGS__)
#define FURI_LOG_I(tag, format, ...) \
    furi_log_print_format(FuriLogLevelInfo, tag, format, ##__VA_ARGS__)
#define FURI_LOG_D(tag, format, ...) \
    furi_log_print_format(FuriLogLevelDebug, tag, format, ##__VA_ARGS__)
#define FURI_LOG_T(tag, format, ...) \
    furi_log_print_format(FuriLogLevelTrace, tag, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
