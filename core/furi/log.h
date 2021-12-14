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

void furi_log_init();
void furi_log_print(FuriLogLevel level, const char* format, ...);
void furi_log_set_level(FuriLogLevel level);
FuriLogLevel furi_log_get_level();
void furi_log_set_puts(FuriLogPuts puts);
void furi_log_set_timestamp(FuriLogTimestamp timestamp);

#define FURI_LOG_FORMAT(log_letter, tag, format) \
    FURI_LOG_CLR_##log_letter "[" #log_letter "][" tag "]: " FURI_LOG_CLR_RESET format "\r\n"
#define FURI_LOG_SHOW(tag, format, log_level, log_letter, ...) \
    furi_log_print(log_level, FURI_LOG_FORMAT(log_letter, tag, format), ##__VA_ARGS__)

#define FURI_LOG_E(tag, format, ...) \
    FURI_LOG_SHOW(tag, format, FuriLogLevelError, E, ##__VA_ARGS__)
#define FURI_LOG_W(tag, format, ...) FURI_LOG_SHOW(tag, format, FuriLogLevelWarn, W, ##__VA_ARGS__)
#define FURI_LOG_I(tag, format, ...) FURI_LOG_SHOW(tag, format, FuriLogLevelInfo, I, ##__VA_ARGS__)
#define FURI_LOG_D(tag, format, ...) \
    FURI_LOG_SHOW(tag, format, FuriLogLevelDebug, D, ##__VA_ARGS__)
#define FURI_LOG_T(tag, format, ...) \
    FURI_LOG_SHOW(tag, format, FuriLogLevelTrace, T, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
