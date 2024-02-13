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

#ifndef EXCLUDE_COMMON

#include "str_util.h"
#include "mg_mem.h"
#include "platform.h"

#ifndef C_DISABLE_BUILTIN_SNPRINTF
#define C_DISABLE_BUILTIN_SNPRINTF 1
#endif

#include "mg_mem.h"

size_t c_strnlen(const char* s, size_t maxlen) WEAK;
size_t c_strnlen(const char* s, size_t maxlen) {
    size_t l = 0;
    for(; l < maxlen && s[l] != '\0'; l++) {
    }
    return l;
}

#define C_SNPRINTF_APPEND_CHAR(ch)         \
    do {                                   \
        if(i < (int)buf_size) buf[i] = ch; \
        i++;                               \
    } while(0)

#define C_SNPRINTF_FLAG_ZERO 1

#if C_DISABLE_BUILTIN_SNPRINTF
int c_vsnprintf(char* buf, size_t buf_size, const char* fmt, va_list ap) WEAK;
int c_vsnprintf(char* buf, size_t buf_size, const char* fmt, va_list ap) {
    return vsnprintf(buf, buf_size, fmt, ap);
}
#else
static int c_itoa(char* buf, size_t buf_size, int64_t num, int base, int flags, int field_width) {
    char tmp[40];
    int i = 0, k = 0, neg = 0;

    if(num < 0) {
        neg++;
        num = -num;
    }

    /* Print into temporary buffer - in reverse order */
    do {
        int rem = num % base;
        if(rem < 10) {
            tmp[k++] = '0' + rem;
        } else {
            tmp[k++] = 'a' + (rem - 10);
        }
        num /= base;
    } while(num > 0);

    /* Zero padding */
    if(flags && C_SNPRINTF_FLAG_ZERO) {
        while(k < field_width && k < (int)sizeof(tmp) - 1) {
            tmp[k++] = '0';
        }
    }

    /* And sign */
    if(neg) {
        tmp[k++] = '-';
    }

    /* Now output */
    while(--k >= 0) {
        C_SNPRINTF_APPEND_CHAR(tmp[k]);
    }

    return i;
}

int c_vsnprintf(char* buf, size_t buf_size, const char* fmt, va_list ap) WEAK;
int c_vsnprintf(char* buf, size_t buf_size, const char* fmt, va_list ap) {
    int ch, i = 0, len_mod, flags, precision, field_width;

    while((ch = *fmt++) != '\0') {
        if(ch != '%') {
            C_SNPRINTF_APPEND_CHAR(ch);
        } else {
            /*
       * Conversion specification:
       *   zero or more flags (one of: # 0 - <space> + ')
       *   an optional minimum  field  width (digits)
       *   an  optional precision (. followed by digits, or *)
       *   an optional length modifier (one of: hh h l ll L q j z t)
       *   conversion specifier (one of: d i o u x X e E f F g G a A c s p n)
       */
            flags = field_width = precision = len_mod = 0;

            /* Flags. only zero-pad flag is supported. */
            if(*fmt == '0') {
                flags |= C_SNPRINTF_FLAG_ZERO;
            }

            /* Field width */
            while(*fmt >= '0' && *fmt <= '9') {
                field_width *= 10;
                field_width += *fmt++ - '0';
            }
            /* Dynamic field width */
            if(*fmt == '*') {
                field_width = va_arg(ap, int);
                fmt++;
            }

            /* Precision */
            if(*fmt == '.') {
                fmt++;
                if(*fmt == '*') {
                    precision = va_arg(ap, int);
                    fmt++;
                } else {
                    while(*fmt >= '0' && *fmt <= '9') {
                        precision *= 10;
                        precision += *fmt++ - '0';
                    }
                }
            }

            /* Length modifier */
            switch(*fmt) {
            case 'h':
            case 'l':
            case 'L':
            case 'I':
            case 'q':
            case 'j':
            case 'z':
            case 't':
                len_mod = *fmt++;
                if(*fmt == 'h') {
                    len_mod = 'H';
                    fmt++;
                }
                if(*fmt == 'l') {
                    len_mod = 'q';
                    fmt++;
                }
                break;
            }

            ch = *fmt++;
            if(ch == 's') {
                const char* s = va_arg(ap, const char*); /* Always fetch parameter */
                int j;
                int pad = field_width - (precision >= 0 ? c_strnlen(s, precision) : 0);
                for(j = 0; j < pad; j++) {
                    C_SNPRINTF_APPEND_CHAR(' ');
                }

                /* `s` may be NULL in case of %.*s */
                if(s != NULL) {
                    /* Ignore negative and 0 precisions */
                    for(j = 0; (precision <= 0 || j < precision) && s[j] != '\0'; j++) {
                        C_SNPRINTF_APPEND_CHAR(s[j]);
                    }
                }
            } else if(ch == 'c') {
                ch = va_arg(ap, int); /* Always fetch parameter */
                C_SNPRINTF_APPEND_CHAR(ch);
            } else if(ch == 'd' && len_mod == 0) {
                i += c_itoa(buf + i, buf_size - i, va_arg(ap, int), 10, flags, field_width);
            } else if(ch == 'd' && len_mod == 'l') {
                i += c_itoa(buf + i, buf_size - i, va_arg(ap, long), 10, flags, field_width);
#ifdef SSIZE_MAX
            } else if(ch == 'd' && len_mod == 'z') {
                i += c_itoa(buf + i, buf_size - i, va_arg(ap, ssize_t), 10, flags, field_width);
#endif
            } else if(ch == 'd' && len_mod == 'q') {
                i += c_itoa(buf + i, buf_size - i, va_arg(ap, int64_t), 10, flags, field_width);
            } else if((ch == 'x' || ch == 'u') && len_mod == 0) {
                i += c_itoa(
                    buf + i,
                    buf_size - i,
                    va_arg(ap, unsigned),
                    ch == 'x' ? 16 : 10,
                    flags,
                    field_width);
            } else if((ch == 'x' || ch == 'u') && len_mod == 'l') {
                i += c_itoa(
                    buf + i,
                    buf_size - i,
                    va_arg(ap, unsigned long),
                    ch == 'x' ? 16 : 10,
                    flags,
                    field_width);
            } else if((ch == 'x' || ch == 'u') && len_mod == 'z') {
                i += c_itoa(
                    buf + i,
                    buf_size - i,
                    va_arg(ap, size_t),
                    ch == 'x' ? 16 : 10,
                    flags,
                    field_width);
            } else if(ch == 'p') {
                unsigned long num = (unsigned long)(uintptr_t)va_arg(ap, void*);
                C_SNPRINTF_APPEND_CHAR('0');
                C_SNPRINTF_APPEND_CHAR('x');
                i += c_itoa(buf + i, buf_size - i, num, 16, flags, 0);
            } else {
#ifndef NO_LIBC
                /*
         * TODO(lsm): abort is not nice in a library, remove it
         * Also, ESP8266 SDK doesn't have it
         */
                abort();
#endif
            }
        }
    }

    /* Zero-terminate the result */
    if(buf_size > 0) {
        buf[i < (int)buf_size ? i : (int)buf_size - 1] = '\0';
    }

    return i;
}
#endif

int c_snprintf(char* buf, size_t buf_size, const char* fmt, ...) WEAK;
int c_snprintf(char* buf, size_t buf_size, const char* fmt, ...) {
    int result;
    va_list ap;
    va_start(ap, fmt);
    result = c_vsnprintf(buf, buf_size, fmt, ap);
    va_end(ap);
    return result;
}

#ifdef _WIN32
int to_wchar(const char* path, wchar_t* wbuf, size_t wbuf_len) {
    int ret;
    char buf[MAX_PATH * 2], buf2[MAX_PATH * 2], *p;

    strncpy(buf, path, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    /* Trim trailing slashes. Leave backslash for paths like "X:\" */
    p = buf + strlen(buf) - 1;
    while(p > buf && p[-1] != ':' && (p[0] == '\\' || p[0] == '/')) *p-- = '\0';

    memset(wbuf, 0, wbuf_len * sizeof(wchar_t));
    ret = MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, (int)wbuf_len);

    /*
   * Convert back to Unicode. If doubly-converted string does not match the
   * original, something is fishy, reject.
   */
    WideCharToMultiByte(CP_UTF8, 0, wbuf, (int)wbuf_len, buf2, sizeof(buf2), NULL, NULL);
    if(strcmp(buf, buf2) != 0) {
        wbuf[0] = L'\0';
        ret = 0;
    }

    return ret;
}
#endif /* _WIN32 */

/* The simplest O(mn) algorithm. Better implementation are GPLed */
const char* c_strnstr(const char* s, const char* find, size_t slen) WEAK;
const char* c_strnstr(const char* s, const char* find, size_t slen) {
    size_t find_length = strlen(find);
    size_t i;

    for(i = 0; i < slen; i++) {
        if(i + find_length > slen) {
            return NULL;
        }

        if(strncmp(&s[i], find, find_length) == 0) {
            return &s[i];
        }
    }

    return NULL;
}

#if CS_ENABLE_STRDUP
char* strdup(const char* src) WEAK;
char* strdup(const char* src) {
    size_t len = strlen(src) + 1;
    char* ret = MG_MALLOC(len);
    if(ret != NULL) {
        strcpy(ret, src);
    }
    return ret;
}
#endif

void cs_to_hex(char* to, const unsigned char* p, size_t len) WEAK;
void cs_to_hex(char* to, const unsigned char* p, size_t len) {
    static const char* hex = "0123456789abcdef";

    for(; len--; p++) {
        *to++ = hex[p[0] >> 4];
        *to++ = hex[p[0] & 0x0f];
    }
    *to = '\0';
}

static int fourbit(int ch) {
    if(ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if(ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    } else if(ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return 0;
}

void cs_from_hex(char* to, const char* p, size_t len) WEAK;
void cs_from_hex(char* to, const char* p, size_t len) {
    size_t i;

    for(i = 0; i < len; i += 2) {
        *to++ = (fourbit(p[i]) << 4) + fourbit(p[i + 1]);
    }
    *to = '\0';
}

#if CS_ENABLE_TO64
int64_t cs_to64(const char* s) WEAK;
int64_t cs_to64(const char* s) {
    int64_t result = 0;
    int64_t neg = 1;
    while(*s && isspace((unsigned char)*s)) s++;
    if(*s == '-') {
        neg = -1;
        s++;
    }
    while(isdigit((unsigned char)*s)) {
        result *= 10;
        result += (*s - '0');
        s++;
    }
    return result * neg;
}
#endif

static int str_util_lowercase(const char* s) {
    return tolower(*(const unsigned char*)s);
}

int mg_ncasecmp(const char* s1, const char* s2, size_t len) WEAK;
int mg_ncasecmp(const char* s1, const char* s2, size_t len) {
    int diff = 0;

    if(len > 0) do {
            diff = str_util_lowercase(s1++) - str_util_lowercase(s2++);
        } while(diff == 0 && s1[-1] != '\0' && --len > 0);

    return diff;
}

int mg_casecmp(const char* s1, const char* s2) WEAK;
int mg_casecmp(const char* s1, const char* s2) {
    return mg_ncasecmp(s1, s2, (size_t)~0);
}

int mg_asprintf(char** buf, size_t size, const char* fmt, ...) WEAK;
int mg_asprintf(char** buf, size_t size, const char* fmt, ...) {
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = mg_avprintf(buf, size, fmt, ap);
    va_end(ap);
    return ret;
}

int mg_avprintf(char** buf, size_t size, const char* fmt, va_list ap) WEAK;
int mg_avprintf(char** buf, size_t size, const char* fmt, va_list ap) {
    va_list ap_copy;
    int len;

    va_copy(ap_copy, ap);
    len = vsnprintf(*buf, size, fmt, ap_copy);
    va_end(ap_copy);

    if(len < 0) {
        /* eCos and Windows are not standard-compliant and return -1 when
     * the buffer is too small. Keep allocating larger buffers until we
     * succeed or out of memory. */
        *buf = NULL; /* LCOV_EXCL_START */
        while(len < 0) {
            MG_FREE(*buf);
            if(size == 0) {
                size = 5;
            }
            size *= 2;
            if((*buf = (char*)MG_MALLOC(size)) == NULL) {
                len = -1;
                break;
            }
            va_copy(ap_copy, ap);
            len = vsnprintf(*buf, size - 1, fmt, ap_copy);
            va_end(ap_copy);
        }

        /*
     * Microsoft version of vsnprintf() is not always null-terminated, so put
     * the terminator manually
     */
        (*buf)[len] = 0;
        /* LCOV_EXCL_STOP */
    } else if(len >= (int)size) {
        /* Standard-compliant code path. Allocate a buffer that is large enough. */
        if((*buf = (char*)MG_MALLOC(len + 1)) == NULL) {
            len = -1; /* LCOV_EXCL_LINE */
        } else { /* LCOV_EXCL_LINE */
            va_copy(ap_copy, ap);
            len = vsnprintf(*buf, len + 1, fmt, ap_copy);
            va_end(ap_copy);
        }
    }

    return len;
}

const char* mg_next_comma_list_entry(const char*, struct mg_str*, struct mg_str*) WEAK;
const char* mg_next_comma_list_entry(const char* list, struct mg_str* val, struct mg_str* eq_val) {
    struct mg_str ret = mg_next_comma_list_entry_n(mg_mk_str(list), val, eq_val);
    return ret.p;
}

struct mg_str
    mg_next_comma_list_entry_n(struct mg_str list, struct mg_str* val, struct mg_str* eq_val) WEAK;
struct mg_str
    mg_next_comma_list_entry_n(struct mg_str list, struct mg_str* val, struct mg_str* eq_val) {
    if(list.len == 0) {
        /* End of the list */
        list = mg_mk_str(NULL);
    } else {
        const char* chr = NULL;
        *val = list;

        if((chr = mg_strchr(*val, ',')) != NULL) {
            /* Comma found. Store length and shift the list ptr */
            val->len = chr - val->p;
            chr++;
            list.len -= (chr - list.p);
            list.p = chr;
        } else {
            /* This value is the last one */
            list = mg_mk_str_n(list.p + list.len, 0);
        }

        if(eq_val != NULL) {
            /* Value has form "x=y", adjust pointers and lengths */
            /* so that val points to "x", and eq_val points to "y". */
            eq_val->len = 0;
            eq_val->p = (const char*)memchr(val->p, '=', val->len);
            if(eq_val->p != NULL) {
                eq_val->p++; /* Skip over '=' character */
                eq_val->len = val->p + val->len - eq_val->p;
                val->len = (eq_val->p - val->p) - 1;
            }
        }
    }

    return list;
}

size_t mg_match_prefix_n(const struct mg_str, const struct mg_str) WEAK;
size_t mg_match_prefix_n(const struct mg_str pattern, const struct mg_str str) {
    const char* or_str;
    size_t res = 0, len = 0, i = 0, j = 0;

    if((or_str = (const char*)memchr(pattern.p, '|', pattern.len)) != NULL ||
       (or_str = (const char*)memchr(pattern.p, ',', pattern.len)) != NULL) {
        struct mg_str pstr = {pattern.p, (size_t)(or_str - pattern.p)};
        res = mg_match_prefix_n(pstr, str);
        if(res > 0) return res;
        pstr.p = or_str + 1;
        pstr.len = (pattern.p + pattern.len) - (or_str + 1);
        return mg_match_prefix_n(pstr, str);
    }

    for(; i < pattern.len && j < str.len; i++, j++) {
        if(pattern.p[i] == '?') {
            continue;
        } else if(pattern.p[i] == '*') {
            i++;
            if(i < pattern.len && pattern.p[i] == '*') {
                i++;
                len = str.len - j;
            } else {
                len = 0;
                while(j + len < str.len && str.p[j + len] != '/') len++;
            }
            if(i == pattern.len || (pattern.p[i] == '$' && i == pattern.len - 1)) return j + len;
            do {
                const struct mg_str pstr = {pattern.p + i, pattern.len - i};
                const struct mg_str sstr = {str.p + j + len, str.len - j - len};
                res = mg_match_prefix_n(pstr, sstr);
            } while(res == 0 && len != 0 && len-- > 0);
            return res == 0 ? 0 : j + res + len;
        } else if(str_util_lowercase(&pattern.p[i]) != str_util_lowercase(&str.p[j])) {
            break;
        }
    }
    if(i < pattern.len && pattern.p[i] == '$') {
        return j == str.len ? str.len : 0;
    }
    return i == pattern.len ? j : 0;
}

size_t mg_match_prefix(const char*, int, const char*) WEAK;
size_t mg_match_prefix(const char* pattern, int pattern_len, const char* str) {
    const struct mg_str pstr = {pattern, (size_t)pattern_len};
    struct mg_str s = {str, 0};
    if(str != NULL) s.len = strlen(str);
    return mg_match_prefix_n(pstr, s);
}

#endif /* EXCLUDE_COMMON */
