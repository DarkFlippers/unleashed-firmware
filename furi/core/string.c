#include "string.h"
#include <m-string.h>

struct FuriString {
    string_t string;
};

#undef furi_string_alloc_set
#undef furi_string_set
#undef furi_string_cmp
#undef furi_string_cmpi
#undef furi_string_search
#undef furi_string_search_str
#undef furi_string_equal
#undef furi_string_replace
#undef furi_string_replace_str
#undef furi_string_replace_all
#undef furi_string_start_with
#undef furi_string_end_with
#undef furi_string_end_withi
#undef furi_string_search_char
#undef furi_string_search_rchar
#undef furi_string_trim
#undef furi_string_cat

FuriString* furi_string_alloc(void) {
    FuriString* string = malloc(sizeof(FuriString));
    string_init(string->string);
    return string;
}

FuriString* furi_string_alloc_set(const FuriString* s) {
    FuriString* string = malloc(sizeof(FuriString)); //-V799
    string_init_set(string->string, s->string);
    return string;
} //-V773

FuriString* furi_string_alloc_set_str(const char cstr[]) {
    FuriString* string = malloc(sizeof(FuriString)); //-V799
    string_init_set(string->string, cstr);
    return string;
} //-V773

FuriString* furi_string_alloc_printf(const char format[], ...) {
    va_list args;
    va_start(args, format);
    FuriString* string = furi_string_alloc_vprintf(format, args);
    va_end(args);
    return string;
}

FuriString* furi_string_alloc_vprintf(const char format[], va_list args) {
    FuriString* string = malloc(sizeof(FuriString));
    string_init_vprintf(string->string, format, args);
    return string;
}

FuriString* furi_string_alloc_move(FuriString* s) {
    FuriString* string = malloc(sizeof(FuriString));
    string_init_move(string->string, s->string);
    free(s);
    return string;
}

void furi_string_free(FuriString* s) {
    string_clear(s->string);
    free(s);
}

void furi_string_reserve(FuriString* s, size_t alloc) {
    string_reserve(s->string, alloc);
}

void furi_string_reset(FuriString* s) {
    string_clear(s->string);
    string_init(s->string);
}

void furi_string_swap(FuriString* v1, FuriString* v2) {
    string_swap(v1->string, v2->string);
}

void furi_string_move(FuriString* v1, FuriString* v2) {
    string_clear(v1->string);
    string_init_move(v1->string, v2->string);
    free(v2);
}

size_t furi_string_hash(const FuriString* v) {
    return string_hash(v->string);
}

char furi_string_get_char(const FuriString* v, size_t index) {
    return string_get_char(v->string, index);
}

const char* furi_string_get_cstr(const FuriString* s) {
    return string_get_cstr(s->string);
}

void furi_string_set(FuriString* s, FuriString* source) {
    string_set(s->string, source->string);
}

void furi_string_set_str(FuriString* s, const char cstr[]) {
    string_set(s->string, cstr);
}

void furi_string_set_strn(FuriString* s, const char str[], size_t n) {
    string_set_strn(s->string, str, n);
}

void furi_string_set_char(FuriString* s, size_t index, const char c) {
    string_set_char(s->string, index, c);
}

int furi_string_cmp(const FuriString* s1, const FuriString* s2) {
    return string_cmp(s1->string, s2->string);
}

int furi_string_cmp_str(const FuriString* s1, const char str[]) {
    return string_cmp(s1->string, str);
}

int furi_string_cmpi(const FuriString* v1, const FuriString* v2) {
    return string_cmpi(v1->string, v2->string);
}

int furi_string_cmpi_str(const FuriString* v1, const char p2[]) {
    return string_cmpi_str(v1->string, p2);
}

size_t furi_string_search(const FuriString* v, const FuriString* needle, size_t start) {
    return string_search(v->string, needle->string, start);
}

size_t furi_string_search_str(const FuriString* v, const char needle[], size_t start) {
    return string_search(v->string, needle, start);
}

bool furi_string_equal(const FuriString* v1, const FuriString* v2) {
    return string_equal_p(v1->string, v2->string);
}

bool furi_string_equal_str(const FuriString* v1, const char v2[]) {
    return string_equal_p(v1->string, v2);
}

void furi_string_push_back(FuriString* v, char c) {
    string_push_back(v->string, c);
}

size_t furi_string_size(const FuriString* s) {
    return string_size(s->string);
}

int furi_string_printf(FuriString* v, const char format[], ...) {
    va_list args;
    va_start(args, format);
    int result = furi_string_vprintf(v, format, args);
    va_end(args);
    return result;
}

int furi_string_vprintf(FuriString* v, const char format[], va_list args) {
    return string_vprintf(v->string, format, args);
}

int furi_string_cat_printf(FuriString* v, const char format[], ...) {
    va_list args;
    va_start(args, format);
    int result = furi_string_cat_vprintf(v, format, args);
    va_end(args);
    return result;
}

int furi_string_cat_vprintf(FuriString* v, const char format[], va_list args) {
    FuriString* string = furi_string_alloc();
    int ret = furi_string_vprintf(string, format, args);
    furi_string_cat(v, string);
    furi_string_free(string);
    return ret;
}

bool furi_string_empty(const FuriString* v) {
    return string_empty_p(v->string);
}

void furi_string_replace_at(FuriString* v, size_t pos, size_t len, const char str2[]) {
    string_replace_at(v->string, pos, len, str2);
}

size_t
    furi_string_replace(FuriString* string, FuriString* needle, FuriString* replace, size_t start) {
    return string_replace(string->string, needle->string, replace->string, start);
}

size_t furi_string_replace_str(FuriString* v, const char str1[], const char str2[], size_t start) {
    return string_replace_str(v->string, str1, str2, start);
}

void furi_string_replace_all_str(FuriString* v, const char str1[], const char str2[]) {
    string_replace_all_str(v->string, str1, str2);
}

void furi_string_replace_all(FuriString* v, const FuriString* str1, const FuriString* str2) {
    string_replace_all(v->string, str1->string, str2->string);
}

bool furi_string_start_with(const FuriString* v, const FuriString* v2) {
    return string_start_with_string_p(v->string, v2->string);
}

bool furi_string_start_with_str(const FuriString* v, const char str[]) {
    return string_start_with_str_p(v->string, str);
}

bool furi_string_end_with(const FuriString* v, const FuriString* v2) {
    return string_end_with_string_p(v->string, v2->string);
}

bool furi_string_end_withi(const FuriString* v, const FuriString* v2) {
    return furi_string_end_withi_str(v, string_get_cstr(v2->string));
}

bool furi_string_end_with_str(const FuriString* v, const char str[]) {
    return string_end_with_str_p(v->string, str);
}

bool furi_string_end_withi_str(const FuriString* v, const char str[]) {
    M_STR1NG_CONTRACT(v);
    M_ASSERT(str != NULL);

    const size_t str_len = strlen(str);
    const size_t v_len = string_size(v->string);

    if(v_len < str_len) {
        return false;
    }

    return strcasecmp(&string_get_cstr(v->string)[v_len - str_len], str) == 0;
}

size_t furi_string_search_char(const FuriString* v, char c, size_t start) {
    return string_search_char(v->string, c, start);
}

size_t furi_string_search_rchar(const FuriString* v, char c, size_t start) {
    return string_search_rchar(v->string, c, start);
}

void furi_string_left(FuriString* v, size_t index) {
    string_left(v->string, index);
}

void furi_string_right(FuriString* v, size_t index) {
    string_right(v->string, index);
}

void furi_string_mid(FuriString* v, size_t index, size_t size) {
    string_mid(v->string, index, size);
}

void furi_string_trim(FuriString* v, const char charac[]) {
    string_strim(v->string, charac);
}

void furi_string_cat(FuriString* v, const FuriString* v2) {
    string_cat(v->string, v2->string);
}

void furi_string_cat_str(FuriString* v, const char str[]) {
    string_cat(v->string, str);
}

void furi_string_set_n(FuriString* v, const FuriString* ref, size_t offset, size_t length) {
    string_set_n(v->string, ref->string, offset, length);
}

size_t furi_string_utf8_length(FuriString* str) {
    return string_length_u(str->string);
}

void furi_string_utf8_push(FuriString* str, FuriStringUnicodeValue u) {
    string_push_u(str->string, u);
}

static m_str1ng_utf8_state_e furi_state_to_state(FuriStringUTF8State state) {
    switch(state) {
    case FuriStringUTF8StateStarting:
        return M_STRING_UTF8_STARTING;
    case FuriStringUTF8StateDecoding1:
        return M_STRING_UTF8_DECODING_1;
    case FuriStringUTF8StateDecoding2:
        return M_STRING_UTF8_DECODING_2;
    case FuriStringUTF8StateDecoding3:
        return M_STRING_UTF8_DOCODING_3;
    default:
        return M_STRING_UTF8_ERROR;
    }
}

static FuriStringUTF8State state_to_furi_state(m_str1ng_utf8_state_e state) {
    switch(state) {
    case M_STRING_UTF8_STARTING:
        return FuriStringUTF8StateStarting;
    case M_STRING_UTF8_DECODING_1:
        return FuriStringUTF8StateDecoding1;
    case M_STRING_UTF8_DECODING_2:
        return FuriStringUTF8StateDecoding2;
    case M_STRING_UTF8_DOCODING_3:
        return FuriStringUTF8StateDecoding3;
    default:
        return FuriStringUTF8StateError;
    }
}

void furi_string_utf8_decode(char c, FuriStringUTF8State* state, FuriStringUnicodeValue* unicode) {
    string_unicode_t m_u = *unicode;
    m_str1ng_utf8_state_e m_state = furi_state_to_state(*state);
    m_str1ng_utf8_decode(c, &m_state, &m_u);
    *state = state_to_furi_state(m_state);
    *unicode = m_u;
}
