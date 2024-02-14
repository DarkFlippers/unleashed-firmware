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

#ifndef CS_COMMON_STR_UTIL_H_
#define CS_COMMON_STR_UTIL_H_

#include <stdarg.h>
#include <stdlib.h>

#include "mg_str.h"
#include "platform.h"

#ifndef CS_ENABLE_STRDUP
#define CS_ENABLE_STRDUP 0
#endif

#ifndef CS_ENABLE_TO64
#define CS_ENABLE_TO64 0
#endif

/*
 * Expands to a string representation of its argument: e.g.
 * `CS_STRINGIFY_LIT(5) expands to "5"`
 */
#if !defined(_MSC_VER) || _MSC_VER >= 1900
#define CS_STRINGIFY_LIT(...) #__VA_ARGS__
#else
#define CS_STRINGIFY_LIT(x) #x
#endif

/*
 * Expands to a string representation of its argument, which is allowed
 * to be a macro: e.g.
 *
 * #define FOO 123
 * CS_STRINGIFY_MACRO(FOO)
 *
 * expands to 123.
 */
#define CS_STRINGIFY_MACRO(x) CS_STRINGIFY_LIT(x)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Equivalent of standard `strnlen()`.
 */
size_t c_strnlen(const char* s, size_t maxlen);

/*
 * Equivalent of standard `snprintf()`.
 */
int c_snprintf(char* buf, size_t buf_size, const char* format, ...) PRINTF_LIKE(3, 4);

/*
 * Equivalent of standard `vsnprintf()`.
 */
int c_vsnprintf(char* buf, size_t buf_size, const char* format, va_list ap);

/*
 * Find the first occurrence of find in s, where the search is limited to the
 * first slen characters of s.
 */
const char* c_strnstr(const char* s, const char* find, size_t slen);

/*
 * Stringify binary data. Output buffer size must be 2 * size_of_input + 1
 * because each byte of input takes 2 bytes in string representation
 * plus 1 byte for the terminating \0 character.
 */
void cs_to_hex(char* to, const unsigned char* p, size_t len);

/*
 * Convert stringified binary data back to binary.
 * Does the reverse of `cs_to_hex()`.
 */
void cs_from_hex(char* to, const char* p, size_t len);

#if CS_ENABLE_STRDUP
/*
 * Equivalent of standard `strdup()`, defined if only `CS_ENABLE_STRDUP` is 1.
 */
char* strdup(const char* src);
#endif

#if CS_ENABLE_TO64
#include <stdint.h>
/*
 * Simple string -> int64 conversion routine.
 */
int64_t cs_to64(const char* s);
#endif

/*
 * Cross-platform version of `strncasecmp()`.
 */
int mg_ncasecmp(const char* s1, const char* s2, size_t len);

/*
 * Cross-platform version of `strcasecmp()`.
 */
int mg_casecmp(const char* s1, const char* s2);

/*
 * Prints message to the buffer. If the buffer is large enough to hold the
 * message, it returns buffer. If buffer is to small, it allocates a large
 * enough buffer on heap and returns allocated buffer.
 * This is a supposed use case:
 *
 * ```c
 *    char buf[5], *p = buf;
 *    mg_avprintf(&p, sizeof(buf), "%s", "hi there");
 *    use_p_somehow(p);
 *    if (p != buf) {
 *      free(p);
 *    }
 * ```
 *
 * The purpose of this is to avoid malloc-ing if generated strings are small.
 */
int mg_asprintf(char** buf, size_t size, const char* fmt, ...) PRINTF_LIKE(3, 4);

/* Same as mg_asprintf, but takes varargs list. */
int mg_avprintf(char** buf, size_t size, const char* fmt, va_list ap);

/*
 * A helper function for traversing a comma separated list of values.
 * It returns a list pointer shifted to the next value or NULL if the end
 * of the list found.
 * The value is stored in a val vector. If the value has a form "x=y", then
 * eq_val vector is initialised to point to the "y" part, and val vector length
 * is adjusted to point only to "x".
 * If the list is just a comma separated list of entries, like "aa,bb,cc" then
 * `eq_val` will contain zero-length string.
 *
 * The purpose of this function is to parse comma separated string without
 * any copying/memory allocation.
 */
const char* mg_next_comma_list_entry(const char* list, struct mg_str* val, struct mg_str* eq_val);

/*
 * Like `mg_next_comma_list_entry()`, but takes `list` as `struct mg_str`.
 * NB: Test return value's .p, not .len. On last itreation that yields result
 * .len will be 0 but .p will not. When finished, .p will be NULL.
 */
struct mg_str
    mg_next_comma_list_entry_n(struct mg_str list, struct mg_str* val, struct mg_str* eq_val);

/*
 * Matches 0-terminated string (mg_match_prefix) or string with given length
 * mg_match_prefix_n against a glob pattern. Glob syntax:
 * ```
 * - * matches zero or more characters until a slash character /
 * - ** matches zero or more characters
 * - ? Matches exactly one character which is not a slash /
 * - | or ,  divides alternative patterns
 * - any other character matches itself
 * ```
 * Match is case-insensitive. Return number of bytes matched.
 * Examples:
 * ```
 * mg_match_prefix("a*f", len, "abcdefgh") == 6
 * mg_match_prefix("a*f", len, "abcdexgh") == 0
 * mg_match_prefix("a*f|de*,xy", len, "defgh") == 5
 * mg_match_prefix("?*", len, "abc") == 3
 * mg_match_prefix("?*", len, "") == 0
 * ```
 */
size_t mg_match_prefix(const char* pattern, int pattern_len, const char* str);

/*
 * Like `mg_match_prefix()`, but takes `pattern` and `str` as `struct mg_str`.
 */
size_t mg_match_prefix_n(const struct mg_str pattern, const struct mg_str str);

#ifdef __cplusplus
}
#endif

#endif /* CS_COMMON_STR_UTIL_H_ */
