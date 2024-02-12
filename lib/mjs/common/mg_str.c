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

#include "mg_mem.h"
#include "mg_str.h"
#include "platform.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int mg_ncasecmp(const char* s1, const char* s2, size_t len) WEAK;

struct mg_str mg_mk_str(const char* s) WEAK;
struct mg_str mg_mk_str(const char* s) {
    struct mg_str ret = {s, 0};
    if(s != NULL) ret.len = strlen(s);
    return ret;
}

struct mg_str mg_mk_str_n(const char* s, size_t len) WEAK;
struct mg_str mg_mk_str_n(const char* s, size_t len) {
    struct mg_str ret = {s, len};
    return ret;
}

int mg_vcmp(const struct mg_str* str1, const char* str2) WEAK;
int mg_vcmp(const struct mg_str* str1, const char* str2) {
    size_t n2 = strlen(str2), n1 = str1->len;
    int r = strncmp(str1->p, str2, (n1 < n2) ? n1 : n2);
    if(r == 0) {
        return n1 - n2;
    }
    return r;
}

int mg_vcasecmp(const struct mg_str* str1, const char* str2) WEAK;
int mg_vcasecmp(const struct mg_str* str1, const char* str2) {
    size_t n2 = strlen(str2), n1 = str1->len;
    int r = mg_ncasecmp(str1->p, str2, (n1 < n2) ? n1 : n2);
    if(r == 0) {
        return n1 - n2;
    }
    return r;
}

static struct mg_str mg_strdup_common(const struct mg_str s, int nul_terminate) {
    struct mg_str r = {NULL, 0};
    if(s.len > 0 && s.p != NULL) {
        char* sc = (char*)MG_MALLOC(s.len + (nul_terminate ? 1 : 0));
        if(sc != NULL) {
            memcpy(sc, s.p, s.len);
            if(nul_terminate) sc[s.len] = '\0';
            r.p = sc;
            r.len = s.len;
        }
    }
    return r;
}

struct mg_str mg_strdup(const struct mg_str s) WEAK;
struct mg_str mg_strdup(const struct mg_str s) {
    return mg_strdup_common(s, 0 /* NUL-terminate */);
}

struct mg_str mg_strdup_nul(const struct mg_str s) WEAK;
struct mg_str mg_strdup_nul(const struct mg_str s) {
    return mg_strdup_common(s, 1 /* NUL-terminate */);
}

const char* mg_strchr(const struct mg_str s, int c) WEAK;
const char* mg_strchr(const struct mg_str s, int c) {
    size_t i;
    for(i = 0; i < s.len; i++) {
        if(s.p[i] == c) return &s.p[i];
    }
    return NULL;
}

int mg_strcmp(const struct mg_str str1, const struct mg_str str2) WEAK;
int mg_strcmp(const struct mg_str str1, const struct mg_str str2) {
    size_t i = 0;
    while(i < str1.len && i < str2.len) {
        int c1 = str1.p[i];
        int c2 = str2.p[i];
        if(c1 < c2) return -1;
        if(c1 > c2) return 1;
        i++;
    }
    if(i < str1.len) return 1;
    if(i < str2.len) return -1;
    return 0;
}

int mg_strncmp(const struct mg_str, const struct mg_str, size_t n) WEAK;
int mg_strncmp(const struct mg_str str1, const struct mg_str str2, size_t n) {
    struct mg_str s1 = str1;
    struct mg_str s2 = str2;

    if(s1.len > n) {
        s1.len = n;
    }
    if(s2.len > n) {
        s2.len = n;
    }
    return mg_strcmp(s1, s2);
}

int mg_strcasecmp(const struct mg_str str1, const struct mg_str str2) WEAK;
int mg_strcasecmp(const struct mg_str str1, const struct mg_str str2) {
    size_t i = 0;
    while(i < str1.len && i < str2.len) {
        int c1 = tolower((int)str1.p[i]);
        int c2 = tolower((int)str2.p[i]);
        if(c1 < c2) return -1;
        if(c1 > c2) return 1;
        i++;
    }
    if(i < str1.len) return 1;
    if(i < str2.len) return -1;
    return 0;
}

void mg_strfree(struct mg_str* s) WEAK;
void mg_strfree(struct mg_str* s) {
    char* sp = (char*)s->p;
    s->p = NULL;
    s->len = 0;
    if(sp != NULL) free(sp);
}

const char* mg_strstr(const struct mg_str haystack, const struct mg_str needle) WEAK;
const char* mg_strstr(const struct mg_str haystack, const struct mg_str needle) {
    size_t i;
    if(needle.len > haystack.len) return NULL;
    for(i = 0; i <= haystack.len - needle.len; i++) {
        if(memcmp(haystack.p + i, needle.p, needle.len) == 0) {
            return haystack.p + i;
        }
    }
    return NULL;
}

struct mg_str mg_strstrip(struct mg_str s) WEAK;
struct mg_str mg_strstrip(struct mg_str s) {
    while(s.len > 0 && isspace((int)*s.p)) {
        s.p++;
        s.len--;
    }
    while(s.len > 0 && isspace((int)*(s.p + s.len - 1))) {
        s.len--;
    }
    return s;
}

int mg_str_starts_with(struct mg_str s, struct mg_str prefix) WEAK;
int mg_str_starts_with(struct mg_str s, struct mg_str prefix) {
    const struct mg_str sp = MG_MK_STR_N(s.p, prefix.len);
    if(s.len < prefix.len) return 0;
    return (mg_strcmp(sp, prefix) == 0);
}
