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

#include "cs_dbg.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cs_time.h"
#include "str_util.h"

enum cs_log_level cs_log_level WEAK =
#if CS_ENABLE_DEBUG
    LL_VERBOSE_DEBUG;
#else
    LL_ERROR;
#endif

#if CS_ENABLE_STDIO
static char* s_file_level = NULL;

void cs_log_set_file_level(const char* file_level) WEAK;

FILE* cs_log_file WEAK = NULL;

#if CS_LOG_ENABLE_TS_DIFF
double cs_log_ts WEAK;
#endif

enum cs_log_level cs_log_cur_msg_level WEAK = LL_NONE;

void cs_log_set_file_level(const char* file_level) {
    char* fl = s_file_level;
    if(file_level != NULL) {
        s_file_level = strdup(file_level);
    } else {
        s_file_level = NULL;
    }
    free(fl);
}

int cs_log_print_prefix(enum cs_log_level level, const char* file, int ln) WEAK;
int cs_log_print_prefix(enum cs_log_level level, const char* file, int ln) {
    char prefix[CS_LOG_PREFIX_LEN], *q;
    const char* p;
    size_t fl = 0, ll = 0, pl = 0;

    if(level > cs_log_level && s_file_level == NULL) return 0;

    p = file + strlen(file);

    while(p != file) {
        const char c = *(p - 1);
        if(c == '/' || c == '\\') break;
        p--;
        fl++;
    }

    ll = (ln < 10000 ? (ln < 1000 ? (ln < 100 ? (ln < 10 ? 1 : 2) : 3) : 4) : 5);
    if(fl > (sizeof(prefix) - ll - 2)) fl = (sizeof(prefix) - ll - 2);

    pl = fl + 1 + ll;
    memcpy(prefix, p, fl);
    q = prefix + pl;
    memset(q, ' ', sizeof(prefix) - pl);
    do {
        *(--q) = '0' + (ln % 10);
        ln /= 10;
    } while(ln > 0);
    *(--q) = ':';

    if(s_file_level != NULL) {
        enum cs_log_level pll = cs_log_level;
        struct mg_str fl = mg_mk_str(s_file_level), ps = MG_MK_STR_N(prefix, pl);
        struct mg_str k, v;
        while((fl = mg_next_comma_list_entry_n(fl, &k, &v)).p != NULL) {
            bool yes = !(!mg_str_starts_with(ps, k) || v.len == 0);
            if(!yes) continue;
            pll = (enum cs_log_level)(*v.p - '0');
            break;
        }
        if(level > pll) return 0;
    }

    if(cs_log_file == NULL) cs_log_file = stderr;
    cs_log_cur_msg_level = level;
    fwrite(prefix, 1, sizeof(prefix), cs_log_file);
#if CS_LOG_ENABLE_TS_DIFF
    {
        double now = cs_time();
        fprintf(cs_log_file, "%7u ", (unsigned int)((now - cs_log_ts) * 1000000));
        cs_log_ts = now;
    }
#endif
    return 1;
}

void cs_log_printf(const char* fmt, ...) WEAK;
void cs_log_printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(cs_log_file, fmt, ap);
    va_end(ap);
    fputc('\n', cs_log_file);
    fflush(cs_log_file);
    cs_log_cur_msg_level = LL_NONE;
}

void cs_log_set_file(FILE* file) WEAK;
void cs_log_set_file(FILE* file) {
    cs_log_file = file;
}

#else

int cs_log_print_prefix(enum cs_log_level level, const char* file, int ln) WEAK;
int cs_log_print_prefix(enum cs_log_level level, const char* file, int ln) {
    (void)level;
    (void)file;
    (void)ln;
    return 0;
}

void cs_log_printf(const char* fmt, ...) WEAK;
void cs_log_printf(const char* fmt, ...) {
    (void)fmt;
}

void cs_log_set_file_level(const char* file_level) {
    (void)file_level;
}

#endif /* CS_ENABLE_STDIO */

void cs_log_set_level(enum cs_log_level level) WEAK;
void cs_log_set_level(enum cs_log_level level) {
    cs_log_level = level;
#if CS_LOG_ENABLE_TS_DIFF && CS_ENABLE_STDIO
    cs_log_ts = cs_time();
#endif
}
