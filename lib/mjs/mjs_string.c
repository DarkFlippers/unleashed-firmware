/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include "mjs_string.h"
#include "common/cs_varint.h"
#include "common/mg_str.h"
#include "mjs_core.h"
#include "mjs_internal.h"
#include "mjs_primitive.h"
#include "mjs_util.h"

// No UTF
typedef unsigned short Rune;
static int chartorune(Rune* rune, const char* str) {
    *rune = *(unsigned char*)str;
    return 1;
}
static int runetochar(char* str, Rune* rune) {
    str[0] = (char)*rune;
    return 1;
}

#ifndef MJS_STRING_BUF_RESERVE
#define MJS_STRING_BUF_RESERVE 100
#endif

MJS_PRIVATE size_t unescape(const char* s, size_t len, char* to);

MJS_PRIVATE void embed_string(
    struct mbuf* m,
    size_t offset,
    const char* p,
    size_t len,
    uint8_t /*enum embstr_flags*/ flags);

/* TODO(lsm): NaN payload location depends on endianness, make crossplatform */
#define GET_VAL_NAN_PAYLOAD(v) ((char*)&(v))

int mjs_is_string(mjs_val_t v) {
    uint64_t t = v & MJS_TAG_MASK;
    return t == MJS_TAG_STRING_I || t == MJS_TAG_STRING_F || t == MJS_TAG_STRING_O ||
           t == MJS_TAG_STRING_5 || t == MJS_TAG_STRING_D;
}

mjs_val_t mjs_mk_string(struct mjs* mjs, const char* p, size_t len, int copy) {
    struct mbuf* m;
    mjs_val_t offset, tag = MJS_TAG_STRING_F;
    if(len == 0) {
        /*
     * Zero length for foreign string has a special meaning (that the foreign
     * string is not inlined into mjs_val_t), so when creating a zero-length
     * string, we always assume it'll be owned. Since the length is zero, it
     * doesn't matter anyway.
     */
        copy = 1;
    }
    m = copy ? &mjs->owned_strings : &mjs->foreign_strings;
    offset = m->len;

    if(len == ~((size_t)0)) len = strlen(p);

    if(copy) {
        /* owned string */
        if(len <= 4) {
            char* s = GET_VAL_NAN_PAYLOAD(offset) + 1;
            offset = 0;
            if(p != 0) {
                memcpy(s, p, len);
            }
            s[-1] = len;
            tag = MJS_TAG_STRING_I;
        } else if(len == 5) {
            char* s = GET_VAL_NAN_PAYLOAD(offset);
            offset = 0;
            if(p != 0) {
                memcpy(s, p, len);
            }
            tag = MJS_TAG_STRING_5;
            // } else if ((dict_index = v_find_string_in_dictionary(p, len)) >= 0) {
            //   offset = 0;
            //   GET_VAL_NAN_PAYLOAD(offset)[0] = dict_index;
            //   tag = MJS_TAG_STRING_D;
        } else {
            if(gc_strings_is_gc_needed(mjs)) {
                mjs->need_gc = 1;
            }

            /*
       * Before embedding new string, check if the reallocation is needed.  If
       * so, perform the reallocation by calling `mbuf_resize` manually, since
       * we need to preallocate some extra space (`MJS_STRING_BUF_RESERVE`)
       */
            if((m->len + len) > m->size) {
                char* prev_buf = m->buf;
                mbuf_resize(m, m->len + len + MJS_STRING_BUF_RESERVE);

                /*
         * There is a corner case: when the source pointer is located within
         * the mbuf. In this case, we should adjust the pointer, because it
         * might have just been reallocated.
         */
                if(p >= prev_buf && p < (prev_buf + m->len)) {
                    p += (m->buf - prev_buf);
                }
            }

            embed_string(m, m->len, p, len, EMBSTR_ZERO_TERM);
            tag = MJS_TAG_STRING_O;
        }
    } else {
        /* foreign string */
        if(sizeof(void*) <= 4 && len <= (1 << 15)) {
            /* small foreign strings can fit length and ptr in the mjs_val_t */
            offset = (uint64_t)len << 32 | (uint64_t)(uintptr_t)p;
        } else {
            /* bigger strings need indirection that uses ram */
            size_t pos = m->len;
            size_t llen = cs_varint_llen(len);

            /* allocate space for len and ptr */
            mbuf_insert(m, pos, NULL, llen + sizeof(p));

            cs_varint_encode(len, (uint8_t*)(m->buf + pos), llen);
            memcpy(m->buf + pos + llen, &p, sizeof(p));
        }
        tag = MJS_TAG_STRING_F;
    }

    /* NOTE(lsm): don't use pointer_to_value, 32-bit ptrs will truncate */
    return (offset & ~MJS_TAG_MASK) | tag;
}

/* Get a pointer to string and string length. */
const char* mjs_get_string(struct mjs* mjs, mjs_val_t* v, size_t* sizep) {
    uint64_t tag = v[0] & MJS_TAG_MASK;
    const char* p = NULL;
    size_t size = 0, llen;

    if(!mjs_is_string(*v)) {
        goto clean;
    }

    if(tag == MJS_TAG_STRING_I) {
        p = GET_VAL_NAN_PAYLOAD(*v) + 1;
        size = p[-1];
    } else if(tag == MJS_TAG_STRING_5) {
        p = GET_VAL_NAN_PAYLOAD(*v);
        size = 5;
        // } else if (tag == MJS_TAG_STRING_D) {
        //   int index = ((unsigned char *) GET_VAL_NAN_PAYLOAD(*v))[0];
        //   size = v_dictionary_strings[index].len;
        //   p = v_dictionary_strings[index].p;
    } else if(tag == MJS_TAG_STRING_O) {
        size_t offset = (size_t)gc_string_mjs_val_to_offset(*v);
        char* s = mjs->owned_strings.buf + offset;
        uint64_t v = 0;
        if(offset < mjs->owned_strings.len &&
           cs_varint_decode((uint8_t*)s, mjs->owned_strings.len - offset, &v, &llen)) {
            size = v;
            p = s + llen;
        } else {
            goto clean;
        }
    } else if(tag == MJS_TAG_STRING_F) {
        /*
     * short foreign strings on <=32-bit machines can be encoded in a compact
     * form:
     *
     *     7         6        5        4        3        2        1        0
     *  11111111|1111tttt|llllllll|llllllll|ssssssss|ssssssss|ssssssss|ssssssss
     *
     * Strings longer than 2^26 will be indireceted through the foreign_strings
     * mbuf.
     *
     * We don't use a different tag to represent those two cases. Instead, all
     * foreign strings represented with the help of the foreign_strings mbuf
     * will have the upper 16-bits of the payload set to zero. This allows us to
     * represent up to 477 million foreign strings longer than 64k.
     */
        uint16_t len = (*v >> 32) & 0xFFFF;
        if(sizeof(void*) <= 4 && len != 0) {
            size = (size_t)len;
            p = (const char*)(uintptr_t)*v;
        } else {
            size_t offset = (size_t)gc_string_mjs_val_to_offset(*v);
            char* s = mjs->foreign_strings.buf + offset;
            uint64_t v = 0;
            if(offset < mjs->foreign_strings.len &&
               cs_varint_decode((uint8_t*)s, mjs->foreign_strings.len - offset, &v, &llen)) {
                size = v;
                memcpy((char**)&p, s + llen, sizeof(p));
            } else {
                goto clean;
            }
        }
    } else {
        assert(0);
    }

clean:
    if(sizep != NULL) {
        *sizep = size;
    }
    return p;
}

const char* mjs_get_cstring(struct mjs* mjs, mjs_val_t* value) {
    size_t size;
    const char* s = mjs_get_string(mjs, value, &size);
    if(s == NULL) return NULL;
    if(s[size] != 0 || strlen(s) != size) {
        return NULL;
    }
    return s;
}

int mjs_strcmp(struct mjs* mjs, mjs_val_t* a, const char* b, size_t len) {
    size_t n;
    const char* s;
    if(len == (size_t)~0) len = strlen(b);
    s = mjs_get_string(mjs, a, &n);
    if(n != len) {
        return n - len;
    }
    return strncmp(s, b, len);
}

MJS_PRIVATE unsigned long cstr_to_ulong(const char* s, size_t len, int* ok) {
    char* e;
    unsigned long res = strtoul(s, &e, 10);
    *ok = (e == s + len) && len != 0;
    return res;
}

MJS_PRIVATE mjs_err_t str_to_ulong(struct mjs* mjs, mjs_val_t v, int* ok, unsigned long* res) {
    enum mjs_err ret = MJS_OK;
    size_t len = 0;
    const char* p = mjs_get_string(mjs, &v, &len);
    *res = cstr_to_ulong(p, len, ok);

    return ret;
}

MJS_PRIVATE int s_cmp(struct mjs* mjs, mjs_val_t a, mjs_val_t b) {
    size_t a_len, b_len;
    const char *a_ptr, *b_ptr;

    a_ptr = mjs_get_string(mjs, &a, &a_len);
    b_ptr = mjs_get_string(mjs, &b, &b_len);

    if(a_len == b_len) {
        return memcmp(a_ptr, b_ptr, a_len);
    }
    if(a_len > b_len) {
        return 1;
    } else if(a_len < b_len) {
        return -1;
    } else {
        return 0;
    }
}

MJS_PRIVATE mjs_val_t s_concat(struct mjs* mjs, mjs_val_t a, mjs_val_t b) {
    size_t a_len, b_len, res_len;
    const char *a_ptr, *b_ptr, *res_ptr;
    mjs_val_t res;

    /* Find out lengths of both srtings */
    a_ptr = mjs_get_string(mjs, &a, &a_len);
    b_ptr = mjs_get_string(mjs, &b, &b_len);

    /* Create a placeholder string */
    res = mjs_mk_string(mjs, NULL, a_len + b_len, 1);

    /* mjs_mk_string() may have reallocated mbuf - revalidate pointers */
    a_ptr = mjs_get_string(mjs, &a, &a_len);
    b_ptr = mjs_get_string(mjs, &b, &b_len);

    /* Copy strings into the placeholder */
    res_ptr = mjs_get_string(mjs, &res, &res_len);
    memcpy((char*)res_ptr, a_ptr, a_len);
    memcpy((char*)res_ptr + a_len, b_ptr, b_len);

    return res;
}

MJS_PRIVATE void mjs_string_to_case(struct mjs* mjs, bool upper) {
    mjs_val_t ret = MJS_UNDEFINED;
    size_t size;
    const char* s = NULL;

    /* get string from `this` */
    if(!mjs_check_arg(mjs, -1 /*this*/, "this", MJS_TYPE_STRING, NULL)) {
        goto clean;
    }
    s = mjs_get_string(mjs, &mjs->vals.this_obj, &size);

    if(size == 0) {
        ret = mjs_mk_string(mjs, "", 0, 1);
        goto clean;
    }

    char* tmp = malloc(size);
    for(size_t i = 0; i < size; i++) {
        tmp[i] = upper ? toupper(s[i]) : tolower(s[i]);
    }
    ret = mjs_mk_string(mjs, tmp, size, 1);
    free(tmp);

clean:
    mjs_return(mjs, ret);
}

MJS_PRIVATE void mjs_string_to_lower_case(struct mjs* mjs) {
    mjs_string_to_case(mjs, false);
}

MJS_PRIVATE void mjs_string_to_upper_case(struct mjs* mjs) {
    mjs_string_to_case(mjs, true);
}

MJS_PRIVATE void mjs_string_slice(struct mjs* mjs) {
    int nargs = mjs_nargs(mjs);
    mjs_val_t ret = mjs_mk_number(mjs, 0);
    mjs_val_t beginSlice_v = MJS_UNDEFINED;
    mjs_val_t endSlice_v = MJS_UNDEFINED;
    int beginSlice = 0;
    int endSlice = 0;
    size_t size;
    const char* s = NULL;

    /* get string from `this` */
    if(!mjs_check_arg(mjs, -1 /*this*/, "this", MJS_TYPE_STRING, NULL)) {
        goto clean;
    }
    s = mjs_get_string(mjs, &mjs->vals.this_obj, &size);

    /* get idx from arg 0 */
    if(!mjs_check_arg(mjs, 0, "beginSlice", MJS_TYPE_NUMBER, &beginSlice_v)) {
        goto clean;
    }
    beginSlice = mjs_normalize_idx(mjs_get_int(mjs, beginSlice_v), size);

    if(nargs >= 2) {
        /* endSlice is given; use it */
        /* get idx from arg 0 */
        if(!mjs_check_arg(mjs, 1, "endSlice", MJS_TYPE_NUMBER, &endSlice_v)) {
            goto clean;
        }
        endSlice = mjs_normalize_idx(mjs_get_int(mjs, endSlice_v), size);
    } else {
        /* endSlice is not given; assume the end of the string */
        endSlice = size;
    }

    if(endSlice < beginSlice) {
        endSlice = beginSlice;
    }

    ret = mjs_mk_string(mjs, s + beginSlice, endSlice - beginSlice, 1);

clean:
    mjs_return(mjs, ret);
}

MJS_PRIVATE void mjs_string_index_of(struct mjs* mjs) {
    mjs_val_t ret = mjs_mk_number(mjs, -1);
    mjs_val_t substr_v = MJS_UNDEFINED;
    mjs_val_t idx_v = MJS_UNDEFINED;
    int idx = 0;
    const char *str = NULL, *substr = NULL;
    size_t str_len = 0, substr_len = 0;

    if(!mjs_check_arg(mjs, -1 /* this */, "this", MJS_TYPE_STRING, NULL)) {
        goto clean;
    }
    str = mjs_get_string(mjs, &mjs->vals.this_obj, &str_len);

    if(!mjs_check_arg(mjs, 0, "searchValue", MJS_TYPE_STRING, &substr_v)) {
        goto clean;
    }
    substr = mjs_get_string(mjs, &substr_v, &substr_len);
    if(mjs_nargs(mjs) > 1) {
        if(!mjs_check_arg(mjs, 1, "fromIndex", MJS_TYPE_NUMBER, &idx_v)) {
            goto clean;
        }
        idx = mjs_get_int(mjs, idx_v);
        if(idx < 0) idx = 0;
        if((size_t)idx > str_len) idx = str_len;
    }
    {
        const char* substr_p;
        struct mg_str mgstr, mgsubstr;
        mgstr.p = str + idx;
        mgstr.len = str_len - idx;
        mgsubstr.p = substr;
        mgsubstr.len = substr_len;
        substr_p = mg_strstr(mgstr, mgsubstr);
        if(substr_p != NULL) {
            ret = mjs_mk_number(mjs, (int)(substr_p - str));
        }
    }

clean:
    mjs_return(mjs, ret);
}

MJS_PRIVATE void mjs_string_char_code_at(struct mjs* mjs) {
    mjs_val_t ret = MJS_UNDEFINED;
    mjs_val_t idx_v = MJS_UNDEFINED;
    int idx = 0;
    size_t size;
    const char* s = NULL;

    /* get string from `this` */
    if(!mjs_check_arg(mjs, -1 /*this*/, "this", MJS_TYPE_STRING, NULL)) {
        goto clean;
    }
    s = mjs_get_string(mjs, &mjs->vals.this_obj, &size);

    /* get idx from arg 0 */
    if(!mjs_check_arg(mjs, 0, "index", MJS_TYPE_NUMBER, &idx_v)) {
        goto clean;
    }
    idx = mjs_normalize_idx(mjs_get_int(mjs, idx_v), size);
    if(idx >= 0 && idx < (int)size) {
        ret = mjs_mk_number(mjs, ((unsigned char*)s)[idx]);
    }

clean:
    mjs_return(mjs, ret);
}

MJS_PRIVATE void mjs_mkstr(struct mjs* mjs) {
    int nargs = mjs_nargs(mjs);
    mjs_val_t ret = MJS_UNDEFINED;

    char* ptr = NULL;
    int offset = 0;
    int len = 0;
    int copy = 0;

    mjs_val_t ptr_v = MJS_UNDEFINED;
    mjs_val_t offset_v = MJS_UNDEFINED;
    mjs_val_t len_v = MJS_UNDEFINED;
    mjs_val_t copy_v = MJS_UNDEFINED;

    if(nargs == 2) {
        ptr_v = mjs_arg(mjs, 0);
        len_v = mjs_arg(mjs, 1);
    } else if(nargs == 3) {
        ptr_v = mjs_arg(mjs, 0);
        offset_v = mjs_arg(mjs, 1);
        len_v = mjs_arg(mjs, 2);
    } else if(nargs == 4) {
        ptr_v = mjs_arg(mjs, 0);
        offset_v = mjs_arg(mjs, 1);
        len_v = mjs_arg(mjs, 2);
        copy_v = mjs_arg(mjs, 3);
    } else {
        mjs_prepend_errorf(
            mjs,
            MJS_TYPE_ERROR,
            "mkstr takes 2, 3 or 4 arguments: (ptr, len), (ptr, "
            "offset, len) or (ptr, offset, len, copy)");
        goto clean;
    }

    if(!mjs_is_foreign(ptr_v)) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "ptr should be a foreign pointer");
        goto clean;
    }

    if(offset_v != MJS_UNDEFINED && !mjs_is_number(offset_v)) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "offset should be a number");
        goto clean;
    }

    if(!mjs_is_number(len_v)) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "len should be a number");
        goto clean;
    }

    copy = mjs_is_truthy(mjs, copy_v);

    /* all arguments are fine */

    ptr = (char*)mjs_get_ptr(mjs, ptr_v);
    if(offset_v != MJS_UNDEFINED) {
        offset = mjs_get_int(mjs, offset_v);
    }
    len = mjs_get_int(mjs, len_v);

    ret = mjs_mk_string(mjs, ptr + offset, len, copy);

clean:
    mjs_return(mjs, ret);
}

enum unescape_error {
    SLRE_INVALID_HEX_DIGIT,
    SLRE_INVALID_ESC_CHAR,
    SLRE_UNTERM_ESC_SEQ,
};

static int hex(int c) {
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -SLRE_INVALID_HEX_DIGIT;
}

static int nextesc(const char** p) {
    const unsigned char* s = (unsigned char*)(*p)++;
    switch(*s) {
    case 0:
        return -SLRE_UNTERM_ESC_SEQ;
    case 'c':
        ++*p;
        return *s & 31;
    case 'b':
        return '\b';
    case 't':
        return '\t';
    case 'n':
        return '\n';
    case 'v':
        return '\v';
    case 'f':
        return '\f';
    case 'r':
        return '\r';
    case '\\':
        return '\\';
    case 'u':
        if(isxdigit(s[1]) && isxdigit(s[2]) && isxdigit(s[3]) && isxdigit(s[4])) {
            (*p) += 4;
            return hex(s[1]) << 12 | hex(s[2]) << 8 | hex(s[3]) << 4 | hex(s[4]);
        }
        return -SLRE_INVALID_HEX_DIGIT;
    case 'x':
        if(isxdigit(s[1]) && isxdigit(s[2])) {
            (*p) += 2;
            return (hex(s[1]) << 4) | hex(s[2]);
        }
        return -SLRE_INVALID_HEX_DIGIT;
    default:
        return -SLRE_INVALID_ESC_CHAR;
    }
}

MJS_PRIVATE size_t unescape(const char* s, size_t len, char* to) {
    const char* end = s + len;
    size_t n = 0;
    char tmp[4];
    Rune r;

    while(s < end) {
        s += chartorune(&r, s);
        if(r == '\\' && s < end) {
            switch(*s) {
            case '"':
                s++, r = '"';
                break;
            case '\'':
                s++, r = '\'';
                break;
            case '\n':
                s++, r = '\n';
                break;
            default: {
                const char* tmp_s = s;
                int i = nextesc(&s);
                switch(i) {
                case -SLRE_INVALID_ESC_CHAR:
                    r = '\\';
                    s = tmp_s;
                    n += runetochar(to == NULL ? tmp : to + n, &r);
                    s += chartorune(&r, s);
                    break;
                case -SLRE_INVALID_HEX_DIGIT:
                default:
                    r = i;
                }
            }
            }
        }
        n += runetochar(to == NULL ? tmp : to + n, &r);
    }

    return n;
}

MJS_PRIVATE void embed_string(
    struct mbuf* m,
    size_t offset,
    const char* p,
    size_t len,
    uint8_t /*enum embstr_flags*/ flags) {
    char* old_base = m->buf;
    uint8_t p_backed_by_mbuf = p >= old_base && p < old_base + m->len;
    size_t n = (flags & EMBSTR_UNESCAPE) ? unescape(p, len, NULL) : len;

    /* Calculate how many bytes length takes */
    size_t k = cs_varint_llen(n);

    /* total length: varing length + string len + zero-term */
    size_t tot_len = k + n + !!(flags & EMBSTR_ZERO_TERM);

    /* Allocate buffer */
    mbuf_insert(m, offset, NULL, tot_len);

    /* Fixup p if it was relocated by mbuf_insert() above */
    if(p_backed_by_mbuf) {
        p += m->buf - old_base;
    }

    /* Write length */
    cs_varint_encode(n, (unsigned char*)m->buf + offset, k);

    /* Write string */
    if(p != 0) {
        if(flags & EMBSTR_UNESCAPE) {
            unescape(p, len, m->buf + offset + k);
        } else {
            memcpy(m->buf + offset + k, p, len);
        }
    }

    /* add NULL-terminator if needed */
    if(flags & EMBSTR_ZERO_TERM) {
        m->buf[offset + tot_len - 1] = '\0';
    }
}
