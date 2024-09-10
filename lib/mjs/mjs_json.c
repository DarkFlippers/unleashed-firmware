/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#include "common/str_util.h"
#include "common/frozen/frozen.h"
#include "mjs_array.h"
#include "mjs_internal.h"
#include "mjs_core.h"
#include "mjs_object.h"
#include "mjs_primitive.h"
#include "mjs_string.h"
#include "mjs_util_public.h"

#define BUF_LEFT(size, used) (((size_t)(used) < (size)) ? ((size) - (used)) : 0)

/*
 * Returns whether the value of given type should be skipped when generating
 * JSON output
 *
 * So far it always returns 0, but we might add some logic later, if we
 * implement some non-jsonnable objects
 */
static int should_skip_for_json(enum mjs_type type) {
    int ret;
    switch(type) {
    /* All permitted values */
    case MJS_TYPE_NULL:
    case MJS_TYPE_BOOLEAN:
    case MJS_TYPE_NUMBER:
    case MJS_TYPE_STRING:
    case MJS_TYPE_ARRAY_BUF:
    case MJS_TYPE_ARRAY_BUF_VIEW:
    case MJS_TYPE_OBJECT_GENERIC:
    case MJS_TYPE_OBJECT_ARRAY:
        ret = 0;
        break;
    default:
        ret = 1;
        break;
    }
    return ret;
}

static const char* hex_digits = "0123456789abcdef";
static char* append_hex(char* buf, char* limit, uint8_t c) {
    if(buf < limit) *buf++ = 'u';
    if(buf < limit) *buf++ = '0';
    if(buf < limit) *buf++ = '0';
    if(buf < limit) *buf++ = hex_digits[(int)((c >> 4) % 0xf)];
    if(buf < limit) *buf++ = hex_digits[(int)(c & 0xf)];
    return buf;
}

/*
 * Appends quoted s to buf. Any double quote contained in s will be escaped.
 * Returns the number of characters that would have been added,
 * like snprintf.
 * If size is zero it doesn't output anything but keeps counting.
 */
static int snquote(char* buf, size_t size, const char* s, size_t len) {
    char* limit = buf + size;
    const char* end;
    /*
   * String single character escape sequence:
   * http://www.ecma-international.org/ecma-262/6.0/index.html#table-34
   *
   * 0x8 -> \b
   * 0x9 -> \t
   * 0xa -> \n
   * 0xb -> \v
   * 0xc -> \f
   * 0xd -> \r
   */
    const char* specials = "btnvfr";
    size_t i = 0;

    i++;
    if(buf < limit) *buf++ = '"';

    for(end = s + len; s < end; s++) {
        if(*s == '"' || *s == '\\') {
            i++;
            if(buf < limit) *buf++ = '\\';
        } else if(*s >= '\b' && *s <= '\r') {
            i += 2;
            if(buf < limit) *buf++ = '\\';
            if(buf < limit) *buf++ = specials[*s - '\b'];
            continue;
        } else if((unsigned char)*s < '\b' || (*s > '\r' && *s < ' ')) {
            i += 6 /* \uXX XX */;
            if(buf < limit) *buf++ = '\\';
            buf = append_hex(buf, limit, (uint8_t)*s);
            continue;
        }
        i++;
        if(buf < limit) *buf++ = *s;
    }

    i++;
    if(buf < limit) *buf++ = '"';

    if(buf < limit) {
        *buf = '\0';
    } else if(size != 0) {
        /*
     * There is no room for the NULL char, but the size wasn't zero, so we can
     * safely put NULL in the previous byte
     */
        *(buf - 1) = '\0';
    }
    return i;
}

MJS_PRIVATE mjs_err_t to_json_or_debug(
    struct mjs* mjs,
    mjs_val_t v,
    char* buf,
    size_t size,
    size_t* res_len,
    uint8_t is_debug) {
    mjs_val_t el;
    char* vp;
    mjs_err_t rcode = MJS_OK;
    size_t len = 0;
    /*
   * TODO(dfrank) : also push all `mjs_val_t`s that are declared below
   */

    if(size > 0) *buf = '\0';

    if(!is_debug && should_skip_for_json(mjs_get_type(v))) {
        goto clean;
    }

    for(vp = mjs->json_visited_stack.buf;
        vp < mjs->json_visited_stack.buf + mjs->json_visited_stack.len;
        vp += sizeof(mjs_val_t)) {
        if(*(mjs_val_t*)vp == v) {
            len = strlcpy(buf, "[Circular]", size);
            goto clean;
        }
    }

    switch(mjs_get_type(v)) {
    case MJS_TYPE_NULL:
    case MJS_TYPE_BOOLEAN:
    case MJS_TYPE_NUMBER:
    case MJS_TYPE_UNDEFINED:
    case MJS_TYPE_FOREIGN:
    case MJS_TYPE_ARRAY_BUF:
    case MJS_TYPE_ARRAY_BUF_VIEW:
        /* For those types, regular `mjs_to_string()` works */
        {
            /* refactor: mjs_to_string allocates memory every time */
            char* p = NULL;
            int need_free = 0;
            rcode = mjs_to_string(mjs, &v, &p, &len, &need_free);
            c_snprintf(buf, size, "%.*s", (int)len, p);
            if(need_free) {
                free(p);
            }
        }
        goto clean;

    case MJS_TYPE_STRING: {
        /*
       * For strings we can't just use `primitive_to_str()`, because we need
       * quoted value
       */
        size_t n;
        const char* str = mjs_get_string(mjs, &v, &n);
        len = snquote(buf, size, str, n);
        goto clean;
    }

    case MJS_TYPE_OBJECT_FUNCTION:
    case MJS_TYPE_OBJECT_GENERIC: {
        char* b = buf;
        struct mjs_property* prop = NULL;
        struct mjs_object* o = NULL;

        mbuf_append(&mjs->json_visited_stack, (char*)&v, sizeof(v));
        b += c_snprintf(b, BUF_LEFT(size, b - buf), "{");
        o = get_object_struct(v);
        for(prop = o->properties; prop != NULL; prop = prop->next) {
            size_t n;
            const char* s;
            if(!is_debug && should_skip_for_json(mjs_get_type(prop->value))) {
                continue;
            }
            if(b - buf != 1) { /* Not the first property to be printed */
                b += c_snprintf(b, BUF_LEFT(size, b - buf), ",");
            }
            s = mjs_get_string(mjs, &prop->name, &n);
            b += c_snprintf(b, BUF_LEFT(size, b - buf), "\"%.*s\":", (int)n, s);
            {
                size_t tmp = 0;
                rcode =
                    to_json_or_debug(mjs, prop->value, b, BUF_LEFT(size, b - buf), &tmp, is_debug);
                if(rcode != MJS_OK) {
                    goto clean_iter;
                }
                b += tmp;
            }
        }

        b += c_snprintf(b, BUF_LEFT(size, b - buf), "}");
        mjs->json_visited_stack.len -= sizeof(v);

    clean_iter:
        len = b - buf;
        goto clean;
    }
    case MJS_TYPE_OBJECT_ARRAY: {
        int has;
        char* b = buf;
        size_t i, alen = mjs_array_length(mjs, v);
        mbuf_append(&mjs->json_visited_stack, (char*)&v, sizeof(v));
        b += c_snprintf(b, BUF_LEFT(size, b - buf), "[");
        for(i = 0; i < alen; i++) {
            el = mjs_array_get2(mjs, v, i, &has);
            if(has) {
                size_t tmp = 0;
                if(!is_debug && should_skip_for_json(mjs_get_type(el))) {
                    b += c_snprintf(b, BUF_LEFT(size, b - buf), "null");
                } else {
                    rcode = to_json_or_debug(mjs, el, b, BUF_LEFT(size, b - buf), &tmp, is_debug);
                    if(rcode != MJS_OK) {
                        goto clean;
                    }
                }
                b += tmp;
            } else {
                b += c_snprintf(b, BUF_LEFT(size, b - buf), "null");
            }
            if(i != alen - 1) {
                b += c_snprintf(b, BUF_LEFT(size, b - buf), ",");
            }
        }
        b += c_snprintf(b, BUF_LEFT(size, b - buf), "]");
        mjs->json_visited_stack.len -= sizeof(v);
        len = b - buf;
        goto clean;
    }

    case MJS_TYPES_CNT:
        abort();
    }

    abort();

    len = 0; /* for compilers that don't know about abort() */
    goto clean;

clean:
    if(rcode != MJS_OK) {
        len = 0;
    }
    if(res_len != NULL) {
        *res_len = len;
    }
    return rcode;
}

MJS_PRIVATE mjs_err_t
    mjs_json_stringify(struct mjs* mjs, mjs_val_t v, char* buf, size_t size, char** res) {
    mjs_err_t rcode = MJS_OK;
    char* p = buf;
    size_t len;

    to_json_or_debug(mjs, v, buf, size, &len, 0);

    if(len >= size) {
        /* Buffer is not large enough. Allocate a bigger one */
        p = (char*)malloc(len + 1);
        rcode = mjs_json_stringify(mjs, v, p, len + 1, res);
        assert(*res == p);
        goto clean;
    } else {
        *res = p;
        goto clean;
    }

clean:
    /*
   * If we're going to return an error, and we allocated a buffer, then free
   * it. Otherwise, caller should free it.
   */
    if(rcode != MJS_OK && p != buf) {
        free(p);
    }
    return rcode;
}

/*
 * JSON parsing frame: a separate frame is allocated for each nested
 * object/array during parsing
 */
struct json_parse_frame {
    mjs_val_t val;
    struct json_parse_frame* up;
};

/*
 * Context for JSON parsing by means of json_walk()
 */
struct json_parse_ctx {
    struct mjs* mjs;
    mjs_val_t result;
    struct json_parse_frame* frame;
    enum mjs_err rcode;
};

/* Allocate JSON parse frame */
static struct json_parse_frame* alloc_json_frame(struct json_parse_ctx* ctx, mjs_val_t v) {
    struct json_parse_frame* frame =
        (struct json_parse_frame*)calloc(sizeof(struct json_parse_frame), 1);
    frame->val = v;
    mjs_own(ctx->mjs, &frame->val);
    return frame;
}

/* Free JSON parse frame, return the previous one (which may be NULL) */
static struct json_parse_frame*
    free_json_frame(struct json_parse_ctx* ctx, struct json_parse_frame* frame) {
    struct json_parse_frame* up = frame->up;
    mjs_disown(ctx->mjs, &frame->val);
    free(frame);
    return up;
}

/* Callback for json_walk() */
static void frozen_cb(
    void* data,
    const char* name,
    size_t name_len,
    const char* path,
    const struct json_token* token) {
    struct json_parse_ctx* ctx = (struct json_parse_ctx*)data;
    mjs_val_t v = MJS_UNDEFINED;

    (void)path;

    mjs_own(ctx->mjs, &v);

    switch(token->type) {
    case JSON_TYPE_STRING: {
        char* dst;
        if(token->len > 0 && (dst = malloc(token->len)) != NULL) {
            int len = json_unescape(token->ptr, token->len, dst, token->len);
            if(len < 0) {
                mjs_prepend_errorf(ctx->mjs, MJS_TYPE_ERROR, "invalid JSON string");
                break;
            }
            v = mjs_mk_string(ctx->mjs, dst, len, 1 /* copy */);
            free(dst);
        } else {
            /*
         * This branch is for 0-len strings, and for malloc errors
         * TODO(lsm): on malloc error, propagate the error upstream
         */
            v = mjs_mk_string(ctx->mjs, "", 0, 1 /* copy */);
        }
        break;
    }
    case JSON_TYPE_NUMBER:
        v = mjs_mk_number(ctx->mjs, strtod(token->ptr, NULL));
        break;
    case JSON_TYPE_TRUE:
        v = mjs_mk_boolean(ctx->mjs, 1);
        break;
    case JSON_TYPE_FALSE:
        v = mjs_mk_boolean(ctx->mjs, 0);
        break;
    case JSON_TYPE_NULL:
        v = MJS_NULL;
        break;
    case JSON_TYPE_OBJECT_START:
        v = mjs_mk_object(ctx->mjs);
        break;
    case JSON_TYPE_ARRAY_START:
        v = mjs_mk_array(ctx->mjs);
        break;

    case JSON_TYPE_OBJECT_END:
    case JSON_TYPE_ARRAY_END: {
        /* Object or array has finished: deallocate its frame */
        ctx->frame = free_json_frame(ctx, ctx->frame);
    } break;

    default:
        LOG(LL_ERROR, ("Wrong token type %d\n", token->type));
        break;
    }

    if(!mjs_is_undefined(v)) {
        if(name != NULL && name_len != 0) {
            /* Need to define a property on the current object/array */
            if(mjs_is_object(ctx->frame->val)) {
                mjs_set(ctx->mjs, ctx->frame->val, name, name_len, v);
            } else if(mjs_is_array(ctx->frame->val)) {
                /*
         * TODO(dfrank): consult name_len. Currently it's not a problem due to
         * the implementation details of frozen, but it might change
         */
                int idx = (int)strtod(name, NULL);
                mjs_array_set(ctx->mjs, ctx->frame->val, idx, v);
            } else {
                LOG(LL_ERROR, ("Current value is neither object nor array\n"));
            }
        } else {
            /* This is a root value */
            assert(ctx->frame == NULL);

            /*
       * This value will also be the overall result of JSON parsing
       * (it's already owned by the `mjs_alt_json_parse()`)
       */
            ctx->result = v;
        }

        if(token->type == JSON_TYPE_OBJECT_START || token->type == JSON_TYPE_ARRAY_START) {
            /* New object or array has just started, so we need to allocate a frame
       * for it */
            struct json_parse_frame* new_frame = alloc_json_frame(ctx, v);
            new_frame->up = ctx->frame;
            ctx->frame = new_frame;
        }
    }

    mjs_disown(ctx->mjs, &v);
}

MJS_PRIVATE mjs_err_t mjs_json_parse(struct mjs* mjs, const char* str, size_t len, mjs_val_t* res) {
    struct json_parse_ctx* ctx = (struct json_parse_ctx*)calloc(sizeof(struct json_parse_ctx), 1);
    int json_res;
    enum mjs_err rcode = MJS_OK;

    ctx->mjs = mjs;
    ctx->result = MJS_UNDEFINED;
    ctx->frame = NULL;
    ctx->rcode = MJS_OK;

    mjs_own(mjs, &ctx->result);

    {
        /*
     * We have to reallocate the buffer before invoking json_walk, because
     * frozen_cb can create new strings, which can result in the reallocation
     * of mjs string mbuf, invalidating the `str` pointer.
     */
        char* stmp = malloc(len);
        memcpy(stmp, str, len);
        json_res = json_walk(stmp, len, frozen_cb, ctx);
        free(stmp);
        stmp = NULL;

        /* str might have been invalidated, so null it out */
        str = NULL;
    }

    if(ctx->rcode != MJS_OK) {
        rcode = ctx->rcode;
        mjs_prepend_errorf(mjs, rcode, "invalid JSON string");
    } else if(json_res < 0) {
        /* There was an error during parsing */
        rcode = MJS_TYPE_ERROR;
        mjs_prepend_errorf(mjs, rcode, "invalid JSON string");
    } else {
        /* Expression is parsed successfully */
        *res = ctx->result;

        /* There should be no allocated frames */
        assert(ctx->frame == NULL);
    }

    if(rcode != MJS_OK) {
        /* There might be some allocated frames in case of malformed JSON */
        while(ctx->frame != NULL) {
            ctx->frame = free_json_frame(ctx, ctx->frame);
        }
    }

    mjs_disown(mjs, &ctx->result);
    free(ctx);

    return rcode;
}

MJS_PRIVATE void mjs_op_json_stringify(struct mjs* mjs) {
    mjs_val_t ret = MJS_UNDEFINED;
    mjs_val_t val = mjs_arg(mjs, 0);

    if(mjs_nargs(mjs) < 1) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "missing a value to stringify");
    } else {
        char* p = NULL;
        if(mjs_json_stringify(mjs, val, NULL, 0, &p) == MJS_OK) {
            ret = mjs_mk_string(mjs, p, ~0, 1 /* copy */);
            free(p);
        }
    }

    mjs_return(mjs, ret);
}

MJS_PRIVATE void mjs_op_json_parse(struct mjs* mjs) {
    mjs_val_t ret = MJS_UNDEFINED;
    mjs_val_t arg0 = mjs_arg(mjs, 0);

    if(mjs_is_string(arg0)) {
        size_t len;
        const char* str = mjs_get_string(mjs, &arg0, &len);
        mjs_json_parse(mjs, str, len, &ret);
    } else {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "string argument required");
    }

    mjs_return(mjs, ret);
}
