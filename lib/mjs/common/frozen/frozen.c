/*
 * Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
 * Copyright (c) 2018 Cesanta Software Limited
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

#define _CRT_SECURE_NO_WARNINGS /* Disable deprecation warning in VS2005+ */

#include "frozen.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(WEAK)
#if(defined(__GNUC__) || defined(__TI_COMPILER_VERSION__)) && !defined(_WIN32)
#define WEAK __attribute__((weak))
#else
#define WEAK
#endif
#endif

#ifdef _WIN32
#undef snprintf
#undef vsnprintf
#define snprintf  cs_win_snprintf
#define vsnprintf cs_win_vsnprintf
int cs_win_snprintf(char* str, size_t size, const char* format, ...);
int cs_win_vsnprintf(char* str, size_t size, const char* format, va_list ap);
#if _MSC_VER >= 1700
#include <stdint.h>
#else
typedef _int64 int64_t;
typedef unsigned _int64 uint64_t;
#endif
#define PRId64 "I64d"
#define PRIu64 "I64u"
#else /* _WIN32 */
/* <inttypes.h> wants this for C++ */
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#endif /* _WIN32 */

#ifndef INT64_FMT
#define INT64_FMT PRId64
#endif
#ifndef UINT64_FMT
#define UINT64_FMT PRIu64
#endif

#ifndef va_copy
#define va_copy(x, y) x = y
#endif

#ifndef JSON_ENABLE_ARRAY
#define JSON_ENABLE_ARRAY 1
#endif

struct frozen {
    const char* end;
    const char* cur;

    const char* cur_name;
    size_t cur_name_len;

    /* For callback API */
    char path[JSON_MAX_PATH_LEN];
    size_t path_len;
    void* callback_data;
    json_walk_callback_t callback;
};

struct fstate {
    const char* ptr;
    size_t path_len;
};

#define SET_STATE(fr, ptr, str, len)                \
    struct fstate fstate = {(ptr), (fr)->path_len}; \
    json_append_to_path((fr), (str), (len));

#define CALL_BACK(fr, tok, value, len)                                                         \
    do {                                                                                       \
        if((fr)->callback && ((fr)->path_len == 0 || (fr)->path[(fr)->path_len - 1] != '.')) { \
            struct json_token t = {(value), (int)(len), (tok)};                                \
                                                                                               \
            /* Call the callback with the given value and current name */                      \
            (fr)->callback(                                                                    \
                (fr)->callback_data, (fr)->cur_name, (fr)->cur_name_len, (fr)->path, &t);      \
                                                                                               \
            /* Reset the name */                                                               \
            (fr)->cur_name = NULL;                                                             \
            (fr)->cur_name_len = 0;                                                            \
        }                                                                                      \
    } while(0)

static int json_append_to_path(struct frozen* f, const char* str, int size) {
    int n = f->path_len;
    int left = sizeof(f->path) - n - 1;
    if(size > left) size = left;
    memcpy(f->path + n, str, size);
    f->path[n + size] = '\0';
    f->path_len += size;
    return n;
}

static void json_truncate_path(struct frozen* f, size_t len) {
    f->path_len = len;
    f->path[len] = '\0';
}

static int json_parse_object(struct frozen* f);
static int json_parse_value(struct frozen* f);

#define EXPECT(cond, err_code)         \
    do {                               \
        if(!(cond)) return (err_code); \
    } while(0)

#define TRY(expr)             \
    do {                      \
        int _n = expr;        \
        if(_n < 0) return _n; \
    } while(0)

#define END_OF_STRING (-1)

static int json_left(const struct frozen* f) {
    return f->end - f->cur;
}

static int json_isspace(int ch) {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static void json_skip_whitespaces(struct frozen* f) {
    while(f->cur < f->end && json_isspace(*f->cur))
        f->cur++;
}

static int json_cur(struct frozen* f) {
    json_skip_whitespaces(f);
    return f->cur >= f->end ? END_OF_STRING : *(unsigned char*)f->cur;
}

static int json_test_and_skip(struct frozen* f, int expected) {
    int ch = json_cur(f);
    if(ch == expected) {
        f->cur++;
        return 0;
    }
    return ch == END_OF_STRING ? JSON_STRING_INCOMPLETE : JSON_STRING_INVALID;
}

static int json_isalpha(int ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static int json_isdigit(int ch) {
    return ch >= '0' && ch <= '9';
}

static int json_isxdigit(int ch) {
    return json_isdigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

static int json_get_escape_len(const char* s, int len) {
    switch(*s) {
    case 'u':
        return len < 6 ? JSON_STRING_INCOMPLETE :
               json_isxdigit(s[1]) && json_isxdigit(s[2]) && json_isxdigit(s[3]) &&
                       json_isxdigit(s[4]) ?
                         5 :
                         JSON_STRING_INVALID;
    case '"':
    case '\\':
    case '/':
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
        return len < 2 ? JSON_STRING_INCOMPLETE : 1;
    default:
        return JSON_STRING_INVALID;
    }
}

/* identifier = letter { letter | digit | '_' } */
static int json_parse_identifier(struct frozen* f) {
    EXPECT(json_isalpha(json_cur(f)), JSON_STRING_INVALID);
    {
        SET_STATE(f, f->cur, "", 0);
        while(f->cur < f->end &&
              (*f->cur == '_' || json_isalpha(*f->cur) || json_isdigit(*f->cur))) {
            f->cur++;
        }
        json_truncate_path(f, fstate.path_len);
        CALL_BACK(f, JSON_TYPE_STRING, fstate.ptr, f->cur - fstate.ptr);
    }
    return 0;
}

static int json_get_utf8_char_len(unsigned char ch) {
    if((ch & 0x80) == 0) return 1;
    switch(ch & 0xf0) {
    case 0xf0:
        return 4;
    case 0xe0:
        return 3;
    default:
        return 2;
    }
}

/* string = '"' { quoted_printable_chars } '"' */
static int json_parse_string(struct frozen* f) {
    int n, ch = 0, len = 0;
    TRY(json_test_and_skip(f, '"'));
    {
        SET_STATE(f, f->cur, "", 0);
        for(; f->cur < f->end; f->cur += len) {
            ch = *(unsigned char*)f->cur;
            len = json_get_utf8_char_len((unsigned char)ch);
            EXPECT(ch >= 32 && len > 0, JSON_STRING_INVALID); /* No control chars */
            EXPECT(len <= json_left(f), JSON_STRING_INCOMPLETE);
            if(ch == '\\') {
                EXPECT((n = json_get_escape_len(f->cur + 1, json_left(f))) > 0, n);
                len += n;
            } else if(ch == '"') {
                json_truncate_path(f, fstate.path_len);
                CALL_BACK(f, JSON_TYPE_STRING, fstate.ptr, f->cur - fstate.ptr);
                f->cur++;
                break;
            };
        }
    }
    return ch == '"' ? 0 : JSON_STRING_INCOMPLETE;
}

/* number = [ '-' ] digit+ [ '.' digit+ ] [ ['e'|'E'] ['+'|'-'] digit+ ] */
static int json_parse_number(struct frozen* f) {
    int ch = json_cur(f);
    SET_STATE(f, f->cur, "", 0);
    if(ch == '-') f->cur++;
    EXPECT(f->cur < f->end, JSON_STRING_INCOMPLETE);
    if(f->cur + 1 < f->end && f->cur[0] == '0' && f->cur[1] == 'x') {
        f->cur += 2;
        EXPECT(f->cur < f->end, JSON_STRING_INCOMPLETE);
        EXPECT(json_isxdigit(f->cur[0]), JSON_STRING_INVALID);
        while(f->cur < f->end && json_isxdigit(f->cur[0]))
            f->cur++;
    } else {
        EXPECT(json_isdigit(f->cur[0]), JSON_STRING_INVALID);
        while(f->cur < f->end && json_isdigit(f->cur[0]))
            f->cur++;
        if(f->cur < f->end && f->cur[0] == '.') {
            f->cur++;
            EXPECT(f->cur < f->end, JSON_STRING_INCOMPLETE);
            EXPECT(json_isdigit(f->cur[0]), JSON_STRING_INVALID);
            while(f->cur < f->end && json_isdigit(f->cur[0]))
                f->cur++;
        }
        if(f->cur < f->end && (f->cur[0] == 'e' || f->cur[0] == 'E')) {
            f->cur++;
            EXPECT(f->cur < f->end, JSON_STRING_INCOMPLETE);
            if((f->cur[0] == '+' || f->cur[0] == '-')) f->cur++;
            EXPECT(f->cur < f->end, JSON_STRING_INCOMPLETE);
            EXPECT(json_isdigit(f->cur[0]), JSON_STRING_INVALID);
            while(f->cur < f->end && json_isdigit(f->cur[0]))
                f->cur++;
        }
    }
    json_truncate_path(f, fstate.path_len);
    CALL_BACK(f, JSON_TYPE_NUMBER, fstate.ptr, f->cur - fstate.ptr);
    return 0;
}

#if JSON_ENABLE_ARRAY
/* array = '[' [ value { ',' value } ] ']' */
static int json_parse_array(struct frozen* f) {
    int i = 0, current_path_len;
    char buf[20];
    CALL_BACK(f, JSON_TYPE_ARRAY_START, NULL, 0);
    TRY(json_test_and_skip(f, '['));
    {
        {
            SET_STATE(f, f->cur - 1, "", 0);
            while(json_cur(f) != ']') {
                snprintf(buf, sizeof(buf), "[%d]", i);
                i++;
                current_path_len = json_append_to_path(f, buf, strlen(buf));
                f->cur_name = f->path + strlen(f->path) - strlen(buf) + 1 /*opening brace*/;
                f->cur_name_len = strlen(buf) - 2 /*braces*/;
                TRY(json_parse_value(f));
                json_truncate_path(f, current_path_len);
                if(json_cur(f) == ',') f->cur++;
            }
            TRY(json_test_and_skip(f, ']'));
            json_truncate_path(f, fstate.path_len);
            CALL_BACK(f, JSON_TYPE_ARRAY_END, fstate.ptr, f->cur - fstate.ptr);
        }
    }
    return 0;
}
#endif /* JSON_ENABLE_ARRAY */

static int json_expect(struct frozen* f, const char* s, int len, enum json_token_type tok_type) {
    int i, n = json_left(f);
    SET_STATE(f, f->cur, "", 0);
    for(i = 0; i < len; i++) {
        if(i >= n) return JSON_STRING_INCOMPLETE;
        if(f->cur[i] != s[i]) return JSON_STRING_INVALID;
    }
    f->cur += len;
    json_truncate_path(f, fstate.path_len);

    CALL_BACK(f, tok_type, fstate.ptr, f->cur - fstate.ptr);

    return 0;
}

/* value = 'null' | 'true' | 'false' | number | string | array | object */
static int json_parse_value(struct frozen* f) {
    int ch = json_cur(f);

    switch(ch) {
    case '"':
        TRY(json_parse_string(f));
        break;
    case '{':
        TRY(json_parse_object(f));
        break;
#if JSON_ENABLE_ARRAY
    case '[':
        TRY(json_parse_array(f));
        break;
#endif
    case 'n':
        TRY(json_expect(f, "null", 4, JSON_TYPE_NULL));
        break;
    case 't':
        TRY(json_expect(f, "true", 4, JSON_TYPE_TRUE));
        break;
    case 'f':
        TRY(json_expect(f, "false", 5, JSON_TYPE_FALSE));
        break;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        TRY(json_parse_number(f));
        break;
    default:
        return ch == END_OF_STRING ? JSON_STRING_INCOMPLETE : JSON_STRING_INVALID;
    }

    return 0;
}

/* key = identifier | string */
static int json_parse_key(struct frozen* f) {
    int ch = json_cur(f);
    if(json_isalpha(ch)) {
        TRY(json_parse_identifier(f));
    } else if(ch == '"') {
        TRY(json_parse_string(f));
    } else {
        return ch == END_OF_STRING ? JSON_STRING_INCOMPLETE : JSON_STRING_INVALID;
    }
    return 0;
}

/* pair = key ':' value */
static int json_parse_pair(struct frozen* f) {
    int current_path_len;
    const char* tok;
    json_skip_whitespaces(f);
    tok = f->cur;
    TRY(json_parse_key(f));
    {
        f->cur_name = *tok == '"' ? tok + 1 : tok;
        f->cur_name_len = *tok == '"' ? f->cur - tok - 2 : f->cur - tok;
        current_path_len = json_append_to_path(f, f->cur_name, f->cur_name_len);
    }
    TRY(json_test_and_skip(f, ':'));
    TRY(json_parse_value(f));
    json_truncate_path(f, current_path_len);
    return 0;
}

/* object = '{' pair { ',' pair } '}' */
static int json_parse_object(struct frozen* f) {
    CALL_BACK(f, JSON_TYPE_OBJECT_START, NULL, 0);
    TRY(json_test_and_skip(f, '{'));
    {
        SET_STATE(f, f->cur - 1, ".", 1);
        while(json_cur(f) != '}') {
            TRY(json_parse_pair(f));
            if(json_cur(f) == ',') f->cur++;
        }
        TRY(json_test_and_skip(f, '}'));
        json_truncate_path(f, fstate.path_len);
        CALL_BACK(f, JSON_TYPE_OBJECT_END, fstate.ptr, f->cur - fstate.ptr);
    }
    return 0;
}

static int json_doit(struct frozen* f) {
    if(f->cur == 0 || f->end < f->cur) return JSON_STRING_INVALID;
    if(f->end == f->cur) return JSON_STRING_INCOMPLETE;
    return json_parse_value(f);
}

int json_escape(struct json_out* out, const char* p, size_t len) WEAK;
int json_escape(struct json_out* out, const char* p, size_t len) {
    size_t i, cl, n = 0;
    const char* hex_digits = "0123456789abcdef";
    const char* specials = "btnvfr";

    for(i = 0; i < len; i++) {
        unsigned char ch = ((unsigned char*)p)[i];
        if(ch == '"' || ch == '\\') {
            n += out->printer(out, "\\", 1);
            n += out->printer(out, p + i, 1);
        } else if(ch >= '\b' && ch <= '\r') {
            n += out->printer(out, "\\", 1);
            n += out->printer(out, &specials[ch - '\b'], 1);
        } else if(isprint(ch)) {
            n += out->printer(out, p + i, 1);
        } else if((cl = json_get_utf8_char_len(ch)) == 1) {
            n += out->printer(out, "\\u00", 4);
            n += out->printer(out, &hex_digits[(ch >> 4) % 0xf], 1);
            n += out->printer(out, &hex_digits[ch % 0xf], 1);
        } else {
            n += out->printer(out, p + i, cl);
            i += cl - 1;
        }
    }

    return n;
}

int json_printer_buf(struct json_out* out, const char* buf, size_t len) WEAK;
int json_printer_buf(struct json_out* out, const char* buf, size_t len) {
    size_t avail = out->u.buf.size - out->u.buf.len;
    size_t n = len < avail ? len : avail;
    memcpy(out->u.buf.buf + out->u.buf.len, buf, n);
    out->u.buf.len += n;
    if(out->u.buf.size > 0) {
        size_t idx = out->u.buf.len;
        if(idx >= out->u.buf.size) idx = out->u.buf.size - 1;
        out->u.buf.buf[idx] = '\0';
    }
    return len;
}

int json_printer_file(struct json_out* out, const char* buf, size_t len) WEAK;
int json_printer_file(struct json_out* out, const char* buf, size_t len) {
    return fwrite(buf, 1, len, out->u.fp);
}

#if JSON_ENABLE_BASE64
static int b64idx(int c) {
    if(c < 26) {
        return c + 'A';
    } else if(c < 52) {
        return c - 26 + 'a';
    } else if(c < 62) {
        return c - 52 + '0';
    } else {
        return c == 62 ? '+' : '/';
    }
}

static int b64rev(int c) {
    if(c >= 'A' && c <= 'Z') {
        return c - 'A';
    } else if(c >= 'a' && c <= 'z') {
        return c + 26 - 'a';
    } else if(c >= '0' && c <= '9') {
        return c + 52 - '0';
    } else if(c == '+') {
        return 62;
    } else if(c == '/') {
        return 63;
    } else {
        return 64;
    }
}

static int b64enc(struct json_out* out, const unsigned char* p, int n) {
    char buf[4];
    int i, len = 0;
    for(i = 0; i < n; i += 3) {
        int a = p[i], b = i + 1 < n ? p[i + 1] : 0, c = i + 2 < n ? p[i + 2] : 0;
        buf[0] = b64idx(a >> 2);
        buf[1] = b64idx((a & 3) << 4 | (b >> 4));
        buf[2] = b64idx((b & 15) << 2 | (c >> 6));
        buf[3] = b64idx(c & 63);
        if(i + 1 >= n) buf[2] = '=';
        if(i + 2 >= n) buf[3] = '=';
        len += out->printer(out, buf, sizeof(buf));
    }
    return len;
}

static int b64dec(const char* src, int n, char* dst) {
    const char* end = src + n;
    int len = 0;
    while(src + 3 < end) {
        int a = b64rev(src[0]), b = b64rev(src[1]), c = b64rev(src[2]), d = b64rev(src[3]);
        dst[len++] = (a << 2) | (b >> 4);
        if(src[2] != '=') {
            dst[len++] = (b << 4) | (c >> 2);
            if(src[3] != '=') {
                dst[len++] = (c << 6) | d;
            }
        }
        src += 4;
    }
    return len;
}
#endif /* JSON_ENABLE_BASE64 */

static unsigned char hexdec(const char* s) {
#define HEXTOI(x) (x >= '0' && x <= '9' ? x - '0' : x - 'W')
    int a = tolower(*(const unsigned char*)s);
    int b = tolower(*(const unsigned char*)(s + 1));
    return (HEXTOI(a) << 4) | HEXTOI(b);
}

int json_vprintf(struct json_out* out, const char* fmt, va_list xap) WEAK;
int json_vprintf(struct json_out* out, const char* fmt, va_list xap) {
    int len = 0;
    const char *quote = "\"", *null = "null";
    va_list ap;
    va_copy(ap, xap);

    while(*fmt != '\0') {
        if(strchr(":, \r\n\t[]{}\"", *fmt) != NULL) {
            len += out->printer(out, fmt, 1);
            fmt++;
        } else if(fmt[0] == '%') {
            char buf[21];
            size_t skip = 2;

            if(fmt[1] == 'l' && fmt[2] == 'l' && (fmt[3] == 'd' || fmt[3] == 'u')) {
                int64_t val = va_arg(ap, int64_t);
                const char* fmt2 = fmt[3] == 'u' ? "%" UINT64_FMT : "%" INT64_FMT;
                snprintf(buf, sizeof(buf), fmt2, val);
                len += out->printer(out, buf, strlen(buf));
                skip += 2;
            } else if(fmt[1] == 'z' && fmt[2] == 'u') {
                size_t val = va_arg(ap, size_t);
                snprintf(buf, sizeof(buf), "%lu", (unsigned long)val);
                len += out->printer(out, buf, strlen(buf));
                skip += 1;
            } else if(fmt[1] == 'M') {
                json_printf_callback_t f = va_arg(ap, json_printf_callback_t);
                len += f(out, &ap);
            } else if(fmt[1] == 'B') {
                int val = va_arg(ap, int);
                const char* str = val ? "true" : "false";
                len += out->printer(out, str, strlen(str));
            } else if(fmt[1] == 'H') {
#if JSON_ENABLE_HEX
                const char* hex = "0123456789abcdef";
                int i, n = va_arg(ap, int);
                const unsigned char* p = va_arg(ap, const unsigned char*);
                len += out->printer(out, quote, 1);
                for(i = 0; i < n; i++) {
                    len += out->printer(out, &hex[(p[i] >> 4) & 0xf], 1);
                    len += out->printer(out, &hex[p[i] & 0xf], 1);
                }
                len += out->printer(out, quote, 1);
#endif /* JSON_ENABLE_HEX */
            } else if(fmt[1] == 'V') {
#if JSON_ENABLE_BASE64
                const unsigned char* p = va_arg(ap, const unsigned char*);
                int n = va_arg(ap, int);
                len += out->printer(out, quote, 1);
                len += b64enc(out, p, n);
                len += out->printer(out, quote, 1);
#endif /* JSON_ENABLE_BASE64 */
            } else if(fmt[1] == 'Q' || (fmt[1] == '.' && fmt[2] == '*' && fmt[3] == 'Q')) {
                size_t l = 0;
                const char* p;

                if(fmt[1] == '.') {
                    l = (size_t)va_arg(ap, int);
                    skip += 2;
                }
                p = va_arg(ap, char*);

                if(p == NULL) {
                    len += out->printer(out, null, 4);
                } else {
                    if(fmt[1] == 'Q') {
                        l = strlen(p);
                    }
                    len += out->printer(out, quote, 1);
                    len += json_escape(out, p, l);
                    len += out->printer(out, quote, 1);
                }
            } else {
                /*
         * we delegate printing to the system printf.
         * The goal here is to delegate all modifiers parsing to the system
         * printf, as you can see below we still have to parse the format
         * types.
         *
         * Currently, %s with strings longer than 20 chars will require
         * double-buffering (an auxiliary buffer will be allocated from heap).
         * TODO(dfrank): reimplement %s and %.*s in order to avoid that.
         */

                const char* end_of_format_specifier = "sdfFeEgGlhuIcx.*-0123456789";
                int n = strspn(fmt + 1, end_of_format_specifier);
                char* pbuf = buf;
                int need_len, size = sizeof(buf);
                char fmt2[20];
                va_list ap_copy;
                strlcpy(fmt2, fmt, sizeof(fmt2));

                va_copy(ap_copy, ap);
                need_len = vsnprintf(pbuf, size, fmt2, ap_copy);
                va_end(ap_copy);

                if(need_len < 0) {
                    /*
           * Windows & eCos vsnprintf implementation return -1 on overflow
           * instead of needed size.
           */
                    pbuf = NULL;
                    while(need_len < 0) {
                        free(pbuf);
                        size *= 2;
                        if((pbuf = (char*)malloc(size)) == NULL) break;
                        va_copy(ap_copy, ap);
                        need_len = vsnprintf(pbuf, size, fmt2, ap_copy);
                        va_end(ap_copy);
                    }
                } else if(need_len >= (int)sizeof(buf)) {
                    /*
           * resulting string doesn't fit into a stack-allocated buffer `buf`,
           * so we need to allocate a new buffer from heap and use it
           */
                    if((pbuf = (char*)malloc(need_len + 1)) != NULL) {
                        va_copy(ap_copy, ap);
                        vsnprintf(pbuf, need_len + 1, fmt2, ap_copy);
                        va_end(ap_copy);
                    }
                }
                if(pbuf == NULL) {
                    buf[0] = '\0';
                    pbuf = buf;
                }

                /*
         * however we need to parse the type ourselves in order to advance
         * the va_list by the correct amount; there is no portable way to
         * inherit the advancement made by vprintf.
         * 32-bit (linux or windows) passes va_list by value.
         */
                if((n + 1 == strlen("%" PRId64) && strcmp(fmt2, "%" PRId64) == 0) ||
                   (n + 1 == strlen("%" PRIu64) && strcmp(fmt2, "%" PRIu64) == 0)) {
                    (void)va_arg(ap, int64_t);
                } else if(strcmp(fmt2, "%.*s") == 0) {
                    (void)va_arg(ap, int);
                    (void)va_arg(ap, char*);
                } else {
                    switch(fmt2[n]) {
                    case 'u':
                    case 'd':
                        (void)va_arg(ap, int);
                        break;
                    case 'g':
                    case 'f':
                        (void)va_arg(ap, double);
                        break;
                    case 'p':
                        (void)va_arg(ap, void*);
                        break;
                    default:
                        /* many types are promoted to int */
                        (void)va_arg(ap, int);
                    }
                }

                len += out->printer(out, pbuf, strlen(pbuf));
                skip = n + 1;

                /* If buffer was allocated from heap, free it */
                if(pbuf != buf) {
                    free(pbuf);
                    pbuf = NULL;
                }
            }
            fmt += skip;
        } else if(*fmt == '_' || json_isalpha(*fmt)) {
            len += out->printer(out, quote, 1);
            while(*fmt == '_' || json_isalpha(*fmt) || json_isdigit(*fmt)) {
                len += out->printer(out, fmt, 1);
                fmt++;
            }
            len += out->printer(out, quote, 1);
        } else {
            len += out->printer(out, fmt, 1);
            fmt++;
        }
    }
    va_end(ap);

    return len;
}

int json_printf(struct json_out* out, const char* fmt, ...) WEAK;
int json_printf(struct json_out* out, const char* fmt, ...) {
    int n;
    va_list ap;
    va_start(ap, fmt);
    n = json_vprintf(out, fmt, ap);
    va_end(ap);
    return n;
}

int json_printf_array(struct json_out* out, va_list* ap) WEAK;
int json_printf_array(struct json_out* out, va_list* ap) {
    int len = 0;
    char* arr = va_arg(*ap, char*);
    size_t i, arr_size = va_arg(*ap, size_t);
    size_t elem_size = va_arg(*ap, size_t);
    const char* fmt = va_arg(*ap, char*);
    len += json_printf(out, "[", 1);
    for(i = 0; arr != NULL && i < arr_size / elem_size; i++) {
        union {
            int64_t i;
            double d;
        } val;
        memcpy(&val, arr + i * elem_size, elem_size > sizeof(val) ? sizeof(val) : elem_size);
        if(i > 0) len += json_printf(out, ", ");
        if(strpbrk(fmt, "efg") != NULL) {
            len += json_printf(out, fmt, val.d);
        } else {
            len += json_printf(out, fmt, val.i);
        }
    }
    len += json_printf(out, "]", 1);
    return len;
}

#ifdef _WIN32
int cs_win_vsnprintf(char* str, size_t size, const char* format, va_list ap) WEAK;
int cs_win_vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    int res = _vsnprintf(str, size, format, ap);
    va_end(ap);
    if(res >= size) {
        str[size - 1] = '\0';
    }
    return res;
}

int cs_win_snprintf(char* str, size_t size, const char* format, ...) WEAK;
int cs_win_snprintf(char* str, size_t size, const char* format, ...) {
    int res;
    va_list ap;
    va_start(ap, format);
    res = vsnprintf(str, size, format, ap);
    va_end(ap);
    return res;
}
#endif /* _WIN32 */

int json_walk(
    const char* json_string,
    int json_string_length,
    json_walk_callback_t callback,
    void* callback_data) WEAK;
int json_walk(
    const char* json_string,
    int json_string_length,
    json_walk_callback_t callback,
    void* callback_data) {
    struct frozen frozen;

    memset(&frozen, 0, sizeof(frozen));
    frozen.end = json_string + json_string_length;
    frozen.cur = json_string;
    frozen.callback_data = callback_data;
    frozen.callback = callback;

    TRY(json_doit(&frozen));

    return frozen.cur - json_string;
}

struct scan_array_info {
    int found;
    char path[JSON_MAX_PATH_LEN];
    struct json_token* token;
};

static void json_scanf_array_elem_cb(
    void* callback_data,
    const char* name,
    size_t name_len,
    const char* path,
    const struct json_token* token) {
    struct scan_array_info* info = (struct scan_array_info*)callback_data;

    (void)name;
    (void)name_len;

    if(strcmp(path, info->path) == 0) {
        *info->token = *token;
        info->found = 1;
    }
}

int json_scanf_array_elem(
    const char* s,
    int len,
    const char* path,
    int idx,
    struct json_token* token) WEAK;
int json_scanf_array_elem(
    const char* s,
    int len,
    const char* path,
    int idx,
    struct json_token* token) {
    struct scan_array_info info;
    info.token = token;
    info.found = 0;
    memset(token, 0, sizeof(*token));
    snprintf(info.path, sizeof(info.path), "%s[%d]", path, idx);
    json_walk(s, len, json_scanf_array_elem_cb, &info);
    return info.found ? token->len : -1;
}

struct json_scanf_info {
    int num_conversions;
    char* path;
    const char* fmt;
    void* target;
    void* user_data;
    int type;
};

int json_unescape(const char* src, int slen, char* dst, int dlen) WEAK;
int json_unescape(const char* src, int slen, char* dst, int dlen) {
    char *send = (char*)src + slen, *dend = dst + dlen, *orig_dst = dst, *p;
    const char *esc1 = "\"\\/bfnrt", *esc2 = "\"\\/\b\f\n\r\t";

    while(src < send) {
        if(*src == '\\') {
            if(++src >= send) return JSON_STRING_INCOMPLETE;
            if(*src == 'u') {
                if(send - src < 5) return JSON_STRING_INCOMPLETE;
                /* Here we go: this is a \u.... escape. Process simple one-byte chars */
                if(src[1] == '0' && src[2] == '0') {
                    /* This is \u00xx character from the ASCII range */
                    if(dst < dend) *dst = hexdec(src + 3);
                    src += 4;
                } else {
                    /* Complex \uXX XX escapes drag utf8 lib... Do it at some stage */
                    return JSON_STRING_INVALID;
                }
            } else if((p = (char*)strchr(esc1, *src)) != NULL) {
                if(dst < dend) *dst = esc2[p - esc1];
            } else {
                return JSON_STRING_INVALID;
            }
        } else {
            if(dst < dend) *dst = *src;
        }
        dst++;
        src++;
    }

    return dst - orig_dst;
}

static void json_scanf_cb(
    void* callback_data,
    const char* name,
    size_t name_len,
    const char* path,
    const struct json_token* token) {
    struct json_scanf_info* info = (struct json_scanf_info*)callback_data;
    char buf[32]; /* Must be enough to hold numbers */

    (void)name;
    (void)name_len;

    if(token->ptr == NULL) {
        /*
     * We're not interested here in the events for which we have no value;
     * namely, JSON_TYPE_OBJECT_START and JSON_TYPE_ARRAY_START
     */
        return;
    }

    if(strcmp(path, info->path) != 0) {
        /* It's not the path we're looking for, so, just ignore this callback */
        return;
    }

    switch(info->type) {
    case 'B':
        info->num_conversions++;
        switch(sizeof(bool)) {
        case sizeof(char):
            *(char*)info->target = (token->type == JSON_TYPE_TRUE ? 1 : 0);
            break;
        case sizeof(int):
            *(int*)info->target = (token->type == JSON_TYPE_TRUE ? 1 : 0);
            break;
        default:
            /* should never be here */
            abort();
        }
        break;
    case 'M': {
        union {
            void* p;
            json_scanner_t f;
        } u = {info->target};
        info->num_conversions++;
        u.f(token->ptr, token->len, info->user_data);
        break;
    }
    case 'Q': {
        char** dst = (char**)info->target;
        if(token->type == JSON_TYPE_NULL) {
            *dst = NULL;
        } else {
            int unescaped_len = json_unescape(token->ptr, token->len, NULL, 0);
            if(unescaped_len >= 0 && (*dst = (char*)malloc(unescaped_len + 1)) != NULL) {
                info->num_conversions++;
                if(json_unescape(token->ptr, token->len, *dst, unescaped_len) == unescaped_len) {
                    (*dst)[unescaped_len] = '\0';
                } else {
                    free(*dst);
                    *dst = NULL;
                }
            }
        }
        break;
    }
    case 'H': {
#if JSON_ENABLE_HEX
        char** dst = (char**)info->user_data;
        int i, len = token->len / 2;
        *(int*)info->target = len;
        if((*dst = (char*)malloc(len + 1)) != NULL) {
            for(i = 0; i < len; i++) {
                (*dst)[i] = hexdec(token->ptr + 2 * i);
            }
            (*dst)[len] = '\0';
            info->num_conversions++;
        }
#endif /* JSON_ENABLE_HEX */
        break;
    }
    case 'V': {
#if JSON_ENABLE_BASE64
        char** dst = (char**)info->target;
        int len = token->len * 4 / 3 + 2;
        if((*dst = (char*)malloc(len + 1)) != NULL) {
            int n = b64dec(token->ptr, token->len, *dst);
            (*dst)[n] = '\0';
            *(int*)info->user_data = n;
            info->num_conversions++;
        }
#endif /* JSON_ENABLE_BASE64 */
        break;
    }
    case 'T':
        info->num_conversions++;
        *(struct json_token*)info->target = *token;
        break;
    default:
        if(token->len >= (int)sizeof(buf)) break;
        /* Before converting, copy into tmp buffer in order to 0-terminate it */
        memcpy(buf, token->ptr, token->len);
        buf[token->len] = '\0';
        /* NB: Use of base 0 for %d, %ld, %u and %lu is intentional. */
        if(info->fmt[1] == 'd' || (info->fmt[1] == 'l' && info->fmt[2] == 'd') ||
           info->fmt[1] == 'i') {
            char* endptr = NULL;
            long r = strtol(buf, &endptr, 0 /* base */);
            if(*endptr == '\0') {
                if(info->fmt[1] == 'l') {
                    *((long*)info->target) = r;
                } else {
                    *((int*)info->target) = (int)r;
                }
                info->num_conversions++;
            }
        } else if(info->fmt[1] == 'u' || (info->fmt[1] == 'l' && info->fmt[2] == 'u')) {
            char* endptr = NULL;
            unsigned long r = strtoul(buf, &endptr, 0 /* base */);
            if(*endptr == '\0') {
                if(info->fmt[1] == 'l') {
                    *((unsigned long*)info->target) = r;
                } else {
                    *((unsigned int*)info->target) = (unsigned int)r;
                }
                info->num_conversions++;
            }
        } else {
#if !JSON_MINIMAL
            info->num_conversions += sscanf(buf, info->fmt, info->target);
#endif
        }
        break;
    }
}

int json_vscanf(const char* s, int len, const char* fmt, va_list ap) WEAK;
int json_vscanf(const char* s, int len, const char* fmt, va_list ap) {
    char path[JSON_MAX_PATH_LEN] = "", fmtbuf[20];
    int i = 0;
    char* p = NULL;
    struct json_scanf_info info = {0, path, fmtbuf, NULL, NULL, 0};

    while(fmt[i] != '\0') {
        if(fmt[i] == '{') {
            strlcat(path, ".", sizeof(path));
            i++;
        } else if(fmt[i] == '}') {
            if((p = strrchr(path, '.')) != NULL) *p = '\0';
            i++;
        } else if(fmt[i] == '%') {
            info.target = va_arg(ap, void*);
            info.type = fmt[i + 1];
            switch(fmt[i + 1]) {
            case 'M':
            case 'V':
            case 'H':
                info.user_data = va_arg(ap, void*);
            /* FALLTHROUGH */
            case 'B':
            case 'Q':
            case 'T':
                i += 2;
                break;
            default: {
                const char* delims = ", \t\r\n]}";
                int conv_len = strcspn(fmt + i + 1, delims) + 1;
                memcpy(fmtbuf, fmt + i, conv_len);
                fmtbuf[conv_len] = '\0';
                i += conv_len;
                i += strspn(fmt + i, delims);
                break;
            }
            }
            json_walk(s, len, json_scanf_cb, &info);
        } else if(json_isalpha(fmt[i]) || json_get_utf8_char_len(fmt[i]) > 1) {
            char* pe;
            const char* delims = ": \r\n\t";
            int key_len = strcspn(&fmt[i], delims);
            if((p = strrchr(path, '.')) != NULL) p[1] = '\0';
            pe = path + strlen(path);
            memcpy(pe, fmt + i, key_len);
            pe[key_len] = '\0';
            i += key_len + strspn(fmt + i + key_len, delims);
        } else {
            i++;
        }
    }
    return info.num_conversions;
}

int json_scanf(const char* str, int len, const char* fmt, ...) WEAK;
int json_scanf(const char* str, int len, const char* fmt, ...) {
    int result;
    va_list ap;
    va_start(ap, fmt);
    result = json_vscanf(str, len, fmt, ap);
    va_end(ap);
    return result;
}

int json_vfprintf(const char* file_name, const char* fmt, va_list ap) WEAK;
int json_vfprintf(const char* file_name, const char* fmt, va_list ap) {
    int res = -1;
    FILE* fp = fopen(file_name, "wb");
    if(fp != NULL) {
        struct json_out out = JSON_OUT_FILE(fp);
        res = json_vprintf(&out, fmt, ap);
        fputc('\n', fp);
        fclose(fp);
    }
    return res;
}

int json_fprintf(const char* file_name, const char* fmt, ...) WEAK;
int json_fprintf(const char* file_name, const char* fmt, ...) {
    int result;
    va_list ap;
    va_start(ap, fmt);
    result = json_vfprintf(file_name, fmt, ap);
    va_end(ap);
    return result;
}

char* json_fread(const char* path) WEAK;
char* json_fread(const char* path) {
    FILE* fp;
    char* data = NULL;
    if((fp = fopen(path, "rb")) == NULL) {
    } else if(fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
    } else {
        long size = ftell(fp);
        if(size > 0 && (data = (char*)malloc(size + 1)) != NULL) {
            fseek(fp, 0, SEEK_SET); /* Some platforms might not have rewind(), Oo */
            if(fread(data, 1, size, fp) != (size_t)size) {
                free(data);
                data = NULL;
            } else {
                data[size] = '\0';
            }
        }
        fclose(fp);
    }
    return data;
}

struct json_setf_data {
    const char* json_path;
    const char* base; /* Pointer to the source JSON string */
    int matched; /* Matched part of json_path */
    int pos; /* Offset of the mutated value begin */
    int end; /* Offset of the mutated value end */
    int prev; /* Offset of the previous token end */
};

static int get_matched_prefix_len(const char* s1, const char* s2) {
    int i = 0;
    while(s1[i] && s2[i] && s1[i] == s2[i])
        i++;
    return i;
}

static void json_vsetf_cb(
    void* userdata,
    const char* name,
    size_t name_len,
    const char* path,
    const struct json_token* t) {
    struct json_setf_data* data = (struct json_setf_data*)userdata;
    int off, len = get_matched_prefix_len(path, data->json_path);
    if(t->ptr == NULL) return;
    off = t->ptr - data->base;
    if(len > data->matched) data->matched = len;

    /*
   * If there is no exact path match, set the mutation position to tbe end
   * of the object or array
   */
    if(len < data->matched && data->pos == 0 &&
       (t->type == JSON_TYPE_OBJECT_END || t->type == JSON_TYPE_ARRAY_END)) {
        data->pos = data->end = data->prev;
    }

    /* Exact path match. Set mutation position to the value of this token */
    if(strcmp(path, data->json_path) == 0 && t->type != JSON_TYPE_OBJECT_START &&
       t->type != JSON_TYPE_ARRAY_START) {
        data->pos = off;
        data->end = off + t->len;
    }

    /*
   * For deletion, we need to know where the previous value ends, because
   * we don't know where matched value key starts.
   * When the mutation position is not yet set, remember each value end.
   * When the mutation position is already set, but it is at the beginning
   * of the object/array, we catch the end of the object/array and see
   * whether the object/array start is closer then previously stored prev.
   */
    if(data->pos == 0) {
        data->prev = off + t->len; /* pos is not yet set */
    } else if((t->ptr[0] == '[' || t->ptr[0] == '{') && off + 1 < data->pos && off + 1 > data->prev) {
        data->prev = off + 1;
    }
    (void)name;
    (void)name_len;
}

int json_vsetf(
    const char* s,
    int len,
    struct json_out* out,
    const char* json_path,
    const char* json_fmt,
    va_list ap) WEAK;
int json_vsetf(
    const char* s,
    int len,
    struct json_out* out,
    const char* json_path,
    const char* json_fmt,
    va_list ap) {
    struct json_setf_data data;
    memset(&data, 0, sizeof(data));
    data.json_path = json_path;
    data.base = s;
    data.end = len;
    json_walk(s, len, json_vsetf_cb, &data);
    if(json_fmt == NULL) {
        /* Deletion codepath */
        json_printf(out, "%.*s", data.prev, s);
        /* Trim comma after the value that begins at object/array start */
        if(s[data.prev - 1] == '{' || s[data.prev - 1] == '[') {
            int i = data.end;
            while(i < len && json_isspace(s[i]))
                i++;
            if(s[i] == ',') data.end = i + 1; /* Point after comma */
        }
        json_printf(out, "%.*s", len - data.end, s + data.end);
    } else {
        /* Modification codepath */
        int n, off = data.matched, depth = 0;

        /* Print the unchanged beginning */
        json_printf(out, "%.*s", data.pos, s);

        /* Add missing keys */
        while((n = strcspn(&json_path[off], ".[")) > 0) {
            if(s[data.prev - 1] != '{' && s[data.prev - 1] != '[' && depth == 0) {
                json_printf(out, ",");
            }
            if(off > 0 && json_path[off - 1] != '.') break;
            json_printf(out, "%.*Q:", n, json_path + off);
            off += n;
            if(json_path[off] != '\0') {
                json_printf(out, "%c", json_path[off] == '.' ? '{' : '[');
                depth++;
                off++;
            }
        }
        /* Print the new value */
        json_vprintf(out, json_fmt, ap);

        /* Close brackets/braces of the added missing keys */
        for(; off > data.matched; off--) {
            int ch = json_path[off];
            const char* p = ch == '.' ? "}" : ch == '[' ? "]" : "";
            json_printf(out, "%s", p);
        }

        /* Print the rest of the unchanged string */
        json_printf(out, "%.*s", len - data.end, s + data.end);
    }
    return data.end > data.pos ? 1 : 0;
}

int json_setf(
    const char* s,
    int len,
    struct json_out* out,
    const char* json_path,
    const char* json_fmt,
    ...) WEAK;
int json_setf(
    const char* s,
    int len,
    struct json_out* out,
    const char* json_path,
    const char* json_fmt,
    ...) {
    int result;
    va_list ap;
    va_start(ap, json_fmt);
    result = json_vsetf(s, len, out, json_path, json_fmt, ap);
    va_end(ap);
    return result;
}

struct prettify_data {
    struct json_out* out;
    int level;
    int last_token;
};

static void indent(struct json_out* out, int level) {
    while(level-- > 0)
        out->printer(out, "  ", 2);
}

static void print_key(struct prettify_data* pd, const char* path, const char* name, int name_len) {
    if(pd->last_token != JSON_TYPE_INVALID && pd->last_token != JSON_TYPE_ARRAY_START &&
       pd->last_token != JSON_TYPE_OBJECT_START) {
        pd->out->printer(pd->out, ",", 1);
    }
    if(path[0] != '\0') pd->out->printer(pd->out, "\n", 1);
    indent(pd->out, pd->level);
    if(path[0] != '\0' && path[strlen(path) - 1] != ']') {
        pd->out->printer(pd->out, "\"", 1);
        pd->out->printer(pd->out, name, (int)name_len);
        pd->out->printer(pd->out, "\"", 1);
        pd->out->printer(pd->out, ": ", 2);
    }
}

static void prettify_cb(
    void* userdata,
    const char* name,
    size_t name_len,
    const char* path,
    const struct json_token* t) {
    struct prettify_data* pd = (struct prettify_data*)userdata;
    switch(t->type) {
    case JSON_TYPE_OBJECT_START:
    case JSON_TYPE_ARRAY_START:
        print_key(pd, path, name, name_len);
        pd->out->printer(pd->out, t->type == JSON_TYPE_ARRAY_START ? "[" : "{", 1);
        pd->level++;
        break;
    case JSON_TYPE_OBJECT_END:
    case JSON_TYPE_ARRAY_END:
        pd->level--;
        if(pd->last_token != JSON_TYPE_INVALID && pd->last_token != JSON_TYPE_ARRAY_START &&
           pd->last_token != JSON_TYPE_OBJECT_START) {
            pd->out->printer(pd->out, "\n", 1);
            indent(pd->out, pd->level);
        }
        pd->out->printer(pd->out, t->type == JSON_TYPE_ARRAY_END ? "]" : "}", 1);
        break;
    case JSON_TYPE_NUMBER:
    case JSON_TYPE_NULL:
    case JSON_TYPE_TRUE:
    case JSON_TYPE_FALSE:
    case JSON_TYPE_STRING:
        print_key(pd, path, name, name_len);
        if(t->type == JSON_TYPE_STRING) pd->out->printer(pd->out, "\"", 1);
        pd->out->printer(pd->out, t->ptr, t->len);
        if(t->type == JSON_TYPE_STRING) pd->out->printer(pd->out, "\"", 1);
        break;
    default:
        break;
    }
    pd->last_token = t->type;
}

int json_prettify(const char* s, int len, struct json_out* out) WEAK;
int json_prettify(const char* s, int len, struct json_out* out) {
    struct prettify_data pd = {out, 0, JSON_TYPE_INVALID};
    return json_walk(s, len, prettify_cb, &pd);
}

int json_prettify_file(const char* file_name) WEAK;
int json_prettify_file(const char* file_name) {
    int res = -1;
    char* s = json_fread(file_name);
    FILE* fp;
    if(s != NULL && (fp = fopen(file_name, "wb")) != NULL) {
        struct json_out out = JSON_OUT_FILE(fp);
        res = json_prettify(s, strlen(s), &out);
        if(res < 0) {
            /* On error, restore the old content */
            fclose(fp);
            fp = fopen(file_name, "wb");
            fseek(fp, 0, SEEK_SET);
            fwrite(s, 1, strlen(s), fp);
        } else {
            fputc('\n', fp);
        }
        fclose(fp);
    }
    free(s);
    return res;
}

struct next_data {
    void* handle; // Passed handle. Changed if a next entry is found
    const char* path; // Path to the iterated object/array
    int path_len; // Path length - optimisation
    int found; // Non-0 if found the next entry
    struct json_token* key; // Object's key
    struct json_token* val; // Object's value
    int* idx; // Array index
};

static void next_set_key(struct next_data* d, const char* name, int name_len, int is_array) {
    if(is_array) {
        /* Array. Set index and reset key  */
        if(d->key != NULL) {
            d->key->len = 0;
            d->key->ptr = NULL;
        }
        if(d->idx != NULL) *d->idx = atoi(name);
    } else {
        /* Object. Set key and make index -1 */
        if(d->key != NULL) {
            d->key->ptr = name;
            d->key->len = name_len;
        }
        if(d->idx != NULL) *d->idx = -1;
    }
}

static void json_next_cb(
    void* userdata,
    const char* name,
    size_t name_len,
    const char* path,
    const struct json_token* t) {
    struct next_data* d = (struct next_data*)userdata;
    const char* p = path + d->path_len;
    if(d->found) return;
    if(d->path_len >= (int)strlen(path)) return;
    if(strncmp(d->path, path, d->path_len) != 0) return;
    if(strchr(p + 1, '.') != NULL) return; /* More nested objects - skip */
    if(strchr(p + 1, '[') != NULL) return; /* Ditto for arrays */
    // {OBJECT,ARRAY}_END types do not pass name, _START does. Save key.
    if(t->type == JSON_TYPE_OBJECT_START || t->type == JSON_TYPE_ARRAY_START) {
        next_set_key(d, name, name_len, p[0] == '[');
    } else if(d->handle == NULL || d->handle < (void*)t->ptr) {
        if(t->type != JSON_TYPE_OBJECT_END && t->type != JSON_TYPE_ARRAY_END) {
            next_set_key(d, name, name_len, p[0] == '[');
        }
        if(d->val != NULL) *d->val = *t;
        d->handle = (void*)t->ptr;
        d->found = 1;
    }
}

static void* json_next(
    const char* s,
    int len,
    void* handle,
    const char* path,
    struct json_token* key,
    struct json_token* val,
    int* i) {
    struct json_token tmpval, *v = val == NULL ? &tmpval : val;
    struct json_token tmpkey, *k = key == NULL ? &tmpkey : key;
    int tmpidx, *pidx = i == NULL ? &tmpidx : i;
    struct next_data data = {handle, path, (int)strlen(path), 0, k, v, pidx};
    json_walk(s, len, json_next_cb, &data);
    return data.found ? data.handle : NULL;
}

void* json_next_key(
    const char* s,
    int len,
    void* handle,
    const char* path,
    struct json_token* key,
    struct json_token* val) WEAK;
void* json_next_key(
    const char* s,
    int len,
    void* handle,
    const char* path,
    struct json_token* key,
    struct json_token* val) {
    return json_next(s, len, handle, path, key, val, NULL);
}

void* json_next_elem(
    const char* s,
    int len,
    void* handle,
    const char* path,
    int* idx,
    struct json_token* val) WEAK;
void* json_next_elem(
    const char* s,
    int len,
    void* handle,
    const char* path,
    int* idx,
    struct json_token* val) {
    return json_next(s, len, handle, path, NULL, val, idx);
}

static int json_sprinter(struct json_out* out, const char* str, size_t len) {
    size_t old_len = out->u.buf.buf == NULL ? 0 : strlen(out->u.buf.buf);
    size_t new_len = len + old_len;
    char* p = (char*)realloc(out->u.buf.buf, new_len + 1);
    if(p != NULL) {
        memcpy(p + old_len, str, len);
        p[new_len] = '\0';
        out->u.buf.buf = p;
    }
    return len;
}

char* json_vasprintf(const char* fmt, va_list ap) WEAK;
char* json_vasprintf(const char* fmt, va_list ap) {
    struct json_out out;
    memset(&out, 0, sizeof(out));
    out.printer = json_sprinter;
    json_vprintf(&out, fmt, ap);
    return out.u.buf.buf;
}

char* json_asprintf(const char* fmt, ...) WEAK;
char* json_asprintf(const char* fmt, ...) {
    char* result = NULL;
    va_list ap;
    va_start(ap, fmt);
    result = json_vasprintf(fmt, ap);
    va_end(ap);
    return result;
}
