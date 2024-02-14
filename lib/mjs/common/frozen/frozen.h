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

#ifndef CS_FROZEN_FROZEN_H_
#define CS_FROZEN_FROZEN_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#if defined(_WIN32) && _MSC_VER < 1700
typedef int bool;
enum { false = 0, true = 1 };
#else
#include <stdbool.h>
#endif

/* JSON token type */
enum json_token_type {
    JSON_TYPE_INVALID = 0, /* memsetting to 0 should create INVALID value */
    JSON_TYPE_STRING,
    JSON_TYPE_NUMBER,
    JSON_TYPE_TRUE,
    JSON_TYPE_FALSE,
    JSON_TYPE_NULL,
    JSON_TYPE_OBJECT_START,
    JSON_TYPE_OBJECT_END,
    JSON_TYPE_ARRAY_START,
    JSON_TYPE_ARRAY_END,

    JSON_TYPES_CNT
};

/*
 * Structure containing token type and value. Used in `json_walk()` and
 * `json_scanf()` with the format specifier `%T`.
 */
struct json_token {
    const char* ptr; /* Points to the beginning of the value */
    int len; /* Value length */
    enum json_token_type type; /* Type of the token, possible values are above */
};

#define JSON_INVALID_TOKEN \
    { 0, 0, JSON_TYPE_INVALID }

/* Error codes */
#define JSON_STRING_INVALID -1
#define JSON_STRING_INCOMPLETE -2

/*
 * Callback-based SAX-like API.
 *
 * Property name and length is given only if it's available: i.e. if current
 * event is an object's property. In other cases, `name` is `NULL`. For
 * example, name is never given:
 *   - For the first value in the JSON string;
 *   - For events JSON_TYPE_OBJECT_END and JSON_TYPE_ARRAY_END
 *
 * E.g. for the input `{ "foo": 123, "bar": [ 1, 2, { "baz": true } ] }`,
 * the sequence of callback invocations will be as follows:
 *
 * - type: JSON_TYPE_OBJECT_START, name: NULL, path: "", value: NULL
 * - type: JSON_TYPE_NUMBER, name: "foo", path: ".foo", value: "123"
 * - type: JSON_TYPE_ARRAY_START,  name: "bar", path: ".bar", value: NULL
 * - type: JSON_TYPE_NUMBER, name: "0", path: ".bar[0]", value: "1"
 * - type: JSON_TYPE_NUMBER, name: "1", path: ".bar[1]", value: "2"
 * - type: JSON_TYPE_OBJECT_START, name: "2", path: ".bar[2]", value: NULL
 * - type: JSON_TYPE_TRUE, name: "baz", path: ".bar[2].baz", value: "true"
 * - type: JSON_TYPE_OBJECT_END, name: NULL, path: ".bar[2]", value: "{ \"baz\":
 *true }"
 * - type: JSON_TYPE_ARRAY_END, name: NULL, path: ".bar", value: "[ 1, 2, {
 *\"baz\": true } ]"
 * - type: JSON_TYPE_OBJECT_END, name: NULL, path: "", value: "{ \"foo\": 123,
 *\"bar\": [ 1, 2, { \"baz\": true } ] }"
 */
typedef void (*json_walk_callback_t)(
    void* callback_data,
    const char* name,
    size_t name_len,
    const char* path,
    const struct json_token* token);

/*
 * Parse `json_string`, invoking `callback` in a way similar to SAX parsers;
 * see `json_walk_callback_t`.
 * Return number of processed bytes, or a negative error code.
 */
int json_walk(
    const char* json_string,
    int json_string_length,
    json_walk_callback_t callback,
    void* callback_data);

/*
 * JSON generation API.
 * struct json_out abstracts output, allowing alternative printing plugins.
 */
struct json_out {
    int (*printer)(struct json_out*, const char* str, size_t len);
    union {
        struct {
            char* buf;
            size_t size;
            size_t len;
        } buf;
        void* data;
        FILE* fp;
    } u;
};

extern int json_printer_buf(struct json_out*, const char*, size_t);
extern int json_printer_file(struct json_out*, const char*, size_t);

#define JSON_OUT_BUF(buf, len) \
    {                          \
        json_printer_buf, {    \
            { buf, len, 0 }    \
        }                      \
    }
#define JSON_OUT_FILE(fp)       \
    {                           \
        json_printer_file, {    \
            { (char*)fp, 0, 0 } \
        }                       \
    }

typedef int (*json_printf_callback_t)(struct json_out*, va_list* ap);

/*
 * Generate formatted output into a given sting buffer.
 * This is a superset of printf() function, with extra format specifiers:
 *  - `%B` print json boolean, `true` or `false`. Accepts an `int`.
 *  - `%Q` print quoted escaped string or `null`. Accepts a `const char *`.
 *  - `%.*Q` same as `%Q`, but with length. Accepts `int`, `const char *`
 *  - `%V` print quoted base64-encoded string. Accepts a `const char *`, `int`.
 *  - `%H` print quoted hex-encoded string. Accepts a `int`, `const char *`.
 *  - `%M` invokes a json_printf_callback_t function. That callback function
 *  can consume more parameters.
 *
 * Return number of bytes printed. If the return value is bigger than the
 * supplied buffer, that is an indicator of overflow. In the overflow case,
 * overflown bytes are not printed.
 */
int json_printf(struct json_out*, const char* fmt, ...);
int json_vprintf(struct json_out*, const char* fmt, va_list ap);

/*
 * Same as json_printf, but prints to a file.
 * File is created if does not exist. File is truncated if already exists.
 */
int json_fprintf(const char* file_name, const char* fmt, ...);
int json_vfprintf(const char* file_name, const char* fmt, va_list ap);

/*
 * Print JSON into an allocated 0-terminated string.
 * Return allocated string, or NULL on error.
 * Example:
 *
 * ```c
 *   char *str = json_asprintf("{a:%H}", 3, "abc");
 *   printf("%s\n", str);  // Prints "616263"
 *   free(str);
 * ```
 */
char* json_asprintf(const char* fmt, ...);
char* json_vasprintf(const char* fmt, va_list ap);

/*
 * Helper %M callback that prints contiguous C arrays.
 * Consumes void *array_ptr, size_t array_size, size_t elem_size, char *fmt
 * Return number of bytes printed.
 */
int json_printf_array(struct json_out*, va_list* ap);

/*
 * Scan JSON string `str`, performing scanf-like conversions according to `fmt`.
 * This is a `scanf()` - like function, with following differences:
 *
 * 1. Object keys in the format string may be not quoted, e.g. "{key: %d}"
 * 2. Order of keys in an object is irrelevant.
 * 3. Several extra format specifiers are supported:
 *    - %B: consumes `int *` (or `char *`, if `sizeof(bool) == sizeof(char)`),
 *       expects boolean `true` or `false`.
 *    - %Q: consumes `char **`, expects quoted, JSON-encoded string. Scanned
 *       string is malloc-ed, caller must free() the string.
 *    - %V: consumes `char **`, `int *`. Expects base64-encoded string.
 *       Result string is base64-decoded, malloced and NUL-terminated.
 *       The length of result string is stored in `int *` placeholder.
 *       Caller must free() the result.
 *    - %H: consumes `int *`, `char **`.
 *       Expects a hex-encoded string, e.g. "fa014f".
 *       Result string is hex-decoded, malloced and NUL-terminated.
 *       The length of the result string is stored in `int *` placeholder.
 *       Caller must free() the result.
 *    - %M: consumes custom scanning function pointer and
 *       `void *user_data` parameter - see json_scanner_t definition.
 *    - %T: consumes `struct json_token *`, fills it out with matched token.
 *
 * Return number of elements successfully scanned & converted.
 * Negative number means scan error.
 */
int json_scanf(const char* str, int str_len, const char* fmt, ...);
int json_vscanf(const char* str, int str_len, const char* fmt, va_list ap);

/* json_scanf's %M handler  */
typedef void (*json_scanner_t)(const char* str, int len, void* user_data);

/*
 * Helper function to scan array item with given path and index.
 * Fills `token` with the matched JSON token.
 * Return -1 if no array element found, otherwise non-negative token length.
 */
int json_scanf_array_elem(
    const char* s,
    int len,
    const char* path,
    int index,
    struct json_token* token);

/*
 * Unescape JSON-encoded string src,slen into dst, dlen.
 * src and dst may overlap.
 * If destination buffer is too small (or zero-length), result string is not
 * written but the length is counted nevertheless (similar to snprintf).
 * Return the length of unescaped string in bytes.
 */
int json_unescape(const char* src, int slen, char* dst, int dlen);

/*
 * Escape a string `str`, `str_len` into the printer `out`.
 * Return the number of bytes printed.
 */
int json_escape(struct json_out* out, const char* str, size_t str_len);

/*
 * Read the whole file in memory.
 * Return malloc-ed file content, or NULL on error. The caller must free().
 */
char* json_fread(const char* file_name);

/*
 * Update given JSON string `s,len` by changing the value at given `json_path`.
 * The result is saved to `out`. If `json_fmt` == NULL, that deletes the key.
 * If path is not present, missing keys are added. Array path without an
 * index pushes a value to the end of an array.
 * Return 1 if the string was changed, 0 otherwise.
 *
 * Example:  s is a JSON string { "a": 1, "b": [ 2 ] }
 *   json_setf(s, len, out, ".a", "7");     // { "a": 7, "b": [ 2 ] }
 *   json_setf(s, len, out, ".b", "7");     // { "a": 1, "b": 7 }
 *   json_setf(s, len, out, ".b[]", "7");   // { "a": 1, "b": [ 2,7 ] }
 *   json_setf(s, len, out, ".b", NULL);    // { "a": 1 }
 */
int json_setf(
    const char* s,
    int len,
    struct json_out* out,
    const char* json_path,
    const char* json_fmt,
    ...);

int json_vsetf(
    const char* s,
    int len,
    struct json_out* out,
    const char* json_path,
    const char* json_fmt,
    va_list ap);

/*
 * Pretty-print JSON string `s,len` into `out`.
 * Return number of processed bytes in `s`.
 */
int json_prettify(const char* s, int len, struct json_out* out);

/*
 * Prettify JSON file `file_name`.
 * Return number of processed bytes, or negative number of error.
 * On error, file content is not modified.
 */
int json_prettify_file(const char* file_name);

/*
 * Iterate over an object at given JSON `path`.
 * On each iteration, fill the `key` and `val` tokens. It is OK to pass NULL
 * for `key`, or `val`, in which case they won't be populated.
 * Return an opaque value suitable for the next iteration, or NULL when done.
 *
 * Example:
 *
 * ```c
 * void *h = NULL;
 * struct json_token key, val;
 * while ((h = json_next_key(s, len, h, ".foo", &key, &val)) != NULL) {
 *   printf("[%.*s] -> [%.*s]\n", key.len, key.ptr, val.len, val.ptr);
 * }
 * ```
 */
void* json_next_key(
    const char* s,
    int len,
    void* handle,
    const char* path,
    struct json_token* key,
    struct json_token* val);

/*
 * Iterate over an array at given JSON `path`.
 * Similar to `json_next_key`, but fills array index `idx` instead of `key`.
 */
void* json_next_elem(
    const char* s,
    int len,
    void* handle,
    const char* path,
    int* idx,
    struct json_token* val);

#ifndef JSON_MAX_PATH_LEN
#define JSON_MAX_PATH_LEN 256
#endif

#ifndef JSON_MINIMAL
#define JSON_MINIMAL 0
#endif

#ifndef JSON_ENABLE_BASE64
#define JSON_ENABLE_BASE64 !JSON_MINIMAL
#endif

#ifndef JSON_ENABLE_HEX
#define JSON_ENABLE_HEX !JSON_MINIMAL
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_FROZEN_FROZEN_H_ */
