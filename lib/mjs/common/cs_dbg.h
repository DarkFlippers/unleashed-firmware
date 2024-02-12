/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CS_COMMON_CS_DBG_H_
#define CS_COMMON_CS_DBG_H_

#include "platform.h"

#if CS_ENABLE_STDIO
#include <stdio.h>
#endif

#ifndef CS_ENABLE_DEBUG
#define CS_ENABLE_DEBUG 0
#endif

#ifndef CS_LOG_PREFIX_LEN
#define CS_LOG_PREFIX_LEN 24
#endif

#ifndef CS_LOG_ENABLE_TS_DIFF
#define CS_LOG_ENABLE_TS_DIFF 0
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Log level; `LL_INFO` is the default. Use `cs_log_set_level()` to change it.
 */
enum cs_log_level {
    LL_NONE = -1,
    LL_ERROR = 0,
    LL_WARN = 1,
    LL_INFO = 2,
    LL_DEBUG = 3,
    LL_VERBOSE_DEBUG = 4,

    _LL_MIN = -2,
    _LL_MAX = 5,
};

/*
 * Set max log level to print; messages with the level above the given one will
 * not be printed.
 */
void cs_log_set_level(enum cs_log_level level);

/*
 * A comma-separated set of prefix=level.
 * prefix is matched against the log prefix exactly as printed, including line
 * number, but partial match is ok. Check stops on first matching entry.
 * If nothing matches, default level is used.
 *
 * Examples:
 *   main.c:=4 - everything from main C at verbose debug level.
 *   mongoose.c=1,mjs.c=1,=4 - everything at verbose debug except mg_* and mjs_*
 *
 */
void cs_log_set_file_level(const char* file_level);

/*
 * Helper function which prints message prefix with the given `level`.
 * If message should be printed (according to the current log level
 * and filter), prints the prefix and returns 1, otherwise returns 0.
 *
 * Clients should typically just use `LOG()` macro.
 */
int cs_log_print_prefix(enum cs_log_level level, const char* fname, int line);

extern enum cs_log_level cs_log_level;

#if CS_ENABLE_STDIO

/*
 * Set file to write logs into. If `NULL`, logs go to `stderr`.
 */
void cs_log_set_file(FILE* file);

/*
 * Prints log to the current log file, appends "\n" in the end and flushes the
 * stream.
 */
void cs_log_printf(const char* fmt, ...) PRINTF_LIKE(1, 2);

#if CS_ENABLE_STDIO

/*
 * Format and print message `x` with the given level `l`. Example:
 *
 * ```c
 * LOG(LL_INFO, ("my info message: %d", 123));
 * LOG(LL_DEBUG, ("my debug message: %d", 123));
 * ```
 */
#define LOG(l, x)                                        \
    do {                                                 \
        if(cs_log_print_prefix(l, __FILE__, __LINE__)) { \
            cs_log_printf x;                             \
        }                                                \
    } while(0)

#else

#define LOG(l, x) ((void)l)

#endif

#ifndef CS_NDEBUG

/*
 * Shortcut for `LOG(LL_VERBOSE_DEBUG, (...))`
 */
#define DBG(x) LOG(LL_VERBOSE_DEBUG, x)

#else /* NDEBUG */

#define DBG(x)

#endif

#else /* CS_ENABLE_STDIO */

#define LOG(l, x)
#define DBG(x)

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_COMMON_CS_DBG_H_ */
