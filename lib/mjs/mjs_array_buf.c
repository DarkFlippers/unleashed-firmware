#include "mjs_array_buf.h"
#include "common/cs_varint.h"
#include "common/mg_str.h"
#include "mjs_core.h"
#include "mjs_internal.h"
#include "mjs_primitive.h"
#include "mjs_object.h"
#include "mjs_array.h"
#include "mjs_util.h"
#include "mjs_exec_public.h"

#ifndef MJS_ARRAY_BUF_RESERVE
#define MJS_ARRAY_BUF_RESERVE 100
#endif

#define IS_SIGNED(type) \
    (type == MJS_DATAVIEW_I8 || type == MJS_DATAVIEW_I16 || type == MJS_DATAVIEW_I32)

int mjs_is_array_buf(mjs_val_t v) {
    return (v & MJS_TAG_MASK) == MJS_TAG_ARRAY_BUF;
}

int mjs_is_data_view(mjs_val_t v) {
    return (v & MJS_TAG_MASK) == MJS_TAG_ARRAY_BUF_VIEW;
}

int mjs_is_typed_array(mjs_val_t v) {
    return ((v & MJS_TAG_MASK) == MJS_TAG_ARRAY_BUF) ||
           ((v & MJS_TAG_MASK) == MJS_TAG_ARRAY_BUF_VIEW);
}

char* mjs_array_buf_get_ptr(struct mjs* mjs, mjs_val_t buf, size_t* bytelen) {
    struct mbuf* m = &mjs->array_buffers;
    size_t offset = buf & ~MJS_TAG_MASK;
    char* ptr = m->buf + offset;

    uint64_t len = 0;
    size_t header_len = 0;
    if(offset < m->len && cs_varint_decode((uint8_t*)ptr, m->len - offset, &len, &header_len)) {
        if(bytelen) {
            *bytelen = len;
        }
        return ptr + header_len;
    }

    return NULL;
}

static size_t mjs_dataview_get_element_len(mjs_dataview_type_t type) {
    size_t len = 1;
    switch(type) {
    case MJS_DATAVIEW_U8:
    case MJS_DATAVIEW_I8:
        len = 1;
        break;
    case MJS_DATAVIEW_U16:
    case MJS_DATAVIEW_I16:
        len = 2;
        break;
    case MJS_DATAVIEW_U32:
    case MJS_DATAVIEW_I32:
        len = 4;
        break;
    default:
        break;
    }
    return len;
}

static int64_t get_value(char* buf, mjs_dataview_type_t type) {
    int64_t value = 0;
    switch(type) {
    case MJS_DATAVIEW_U8:
        value = *(uint8_t*)buf;
        break;
    case MJS_DATAVIEW_I8:
        value = *(int8_t*)buf;
        break;
    case MJS_DATAVIEW_U16:
        value = *(uint16_t*)buf;
        break;
    case MJS_DATAVIEW_I16:
        value = *(int16_t*)buf;
        break;
    case MJS_DATAVIEW_U32:
        value = *(uint32_t*)buf;
        break;
    case MJS_DATAVIEW_I32:
        value = *(int32_t*)buf;
        break;
    default:
        break;
    }
    return value;
}

static void set_value(char* buf, int64_t value, mjs_dataview_type_t type) {
    switch(type) {
    case MJS_DATAVIEW_U8:
        *(uint8_t*)buf = (uint8_t)value;
        break;
    case MJS_DATAVIEW_I8:
        *(int8_t*)buf = (int8_t)value;
        break;
    case MJS_DATAVIEW_U16:
        *(uint16_t*)buf = (uint16_t)value;
        break;
    case MJS_DATAVIEW_I16:
        *(int16_t*)buf = (int16_t)value;
        break;
    case MJS_DATAVIEW_U32:
        *(uint32_t*)buf = (uint32_t)value;
        break;
    case MJS_DATAVIEW_I32:
        *(int32_t*)buf = (int32_t)value;
        break;
    default:
        break;
    }
}

static mjs_val_t mjs_dataview_get(struct mjs* mjs, mjs_val_t obj, size_t index) {
    mjs_val_t buf_obj = mjs_get(mjs, obj, "buffer", -1);

    size_t byte_len = 0;
    char* buf = mjs_array_buf_get_ptr(mjs, buf_obj, &byte_len);
    mjs_dataview_type_t type = mjs_get_int(mjs, mjs_get(mjs, obj, "_t", -1));
    if((mjs_dataview_get_element_len(type) * (index + 1)) > byte_len) {
        return MJS_UNDEFINED;
    }

    buf += mjs_dataview_get_element_len(type) * index;
    int64_t value = get_value(buf, type);

    return mjs_mk_number(mjs, value);
}

static mjs_err_t mjs_dataview_set(struct mjs* mjs, mjs_val_t obj, size_t index, int64_t value) {
    mjs_val_t buf_obj = mjs_get(mjs, obj, "buffer", -1);

    size_t byte_len = 0;
    char* buf = mjs_array_buf_get_ptr(mjs, buf_obj, &byte_len);
    mjs_dataview_type_t type = mjs_get_int(mjs, mjs_get(mjs, obj, "_t", -1));
    if((mjs_dataview_get_element_len(type) * (index + 1)) > byte_len) {
        return MJS_TYPE_ERROR;
    }

    buf += mjs_dataview_get_element_len(type) * index;
    set_value(buf, value, type);

    return MJS_OK;
}

mjs_val_t mjs_dataview_get_prop(struct mjs* mjs, mjs_val_t obj, mjs_val_t key) {
    if(!mjs_is_number(key)) {
        return MJS_UNDEFINED;
    }
    int index = mjs_get_int(mjs, key);
    return mjs_dataview_get(mjs, obj, index);
}

mjs_err_t mjs_dataview_set_prop(struct mjs* mjs, mjs_val_t obj, mjs_val_t key, mjs_val_t val) {
    if(!mjs_is_number(key)) {
        return MJS_TYPE_ERROR;
    }
    int index = mjs_get_int(mjs, key);
    int64_t value = 0;

    if(mjs_is_number(val)) {
        value = mjs_get_double(mjs, val);
    } else if(mjs_is_boolean(val)) {
        value = mjs_get_bool(mjs, val) ? (1) : (0);
    }
    return mjs_dataview_set(mjs, obj, index, value);
}

mjs_val_t mjs_dataview_get_buf(struct mjs* mjs, mjs_val_t obj) {
    return mjs_get(mjs, obj, "buffer", -1);
}

mjs_val_t mjs_dataview_get_len(struct mjs* mjs, mjs_val_t obj) {
    size_t bytelen = 0;
    mjs_array_buf_get_ptr(mjs, mjs_dataview_get_buf(mjs, obj), &bytelen);
    mjs_dataview_type_t type = mjs_get_int(mjs, mjs_get(mjs, obj, "_t", -1));
    size_t element_len = mjs_dataview_get_element_len(type);

    return mjs_mk_number(mjs, bytelen / element_len);
}

mjs_val_t mjs_mk_array_buf(struct mjs* mjs, char* data, size_t buf_len) {
    struct mbuf* m = &mjs->array_buffers;

    if((m->len + buf_len) > m->size) {
        char* prev_buf = m->buf;
        mbuf_resize(m, m->len + buf_len + MJS_ARRAY_BUF_RESERVE);

        if(data >= prev_buf && data < (prev_buf + m->len)) {
            data += m->buf - prev_buf;
        }
    }

    size_t offset = m->len;
    char* prev_buf = m->buf;

    size_t header_len = cs_varint_llen(buf_len);
    mbuf_insert(m, offset, NULL, header_len + buf_len);
    if(data >= prev_buf && data < (prev_buf + m->len)) {
        data += m->buf - prev_buf;
    }

    cs_varint_encode(buf_len, (unsigned char*)m->buf + offset, header_len);

    if(data != NULL) {
        memcpy(m->buf + offset + header_len, data, buf_len);
    } else {
        memset(m->buf + offset + header_len, 0, buf_len);
    }

    return (offset & ~MJS_TAG_MASK) | MJS_TAG_ARRAY_BUF;
}

void mjs_array_buf_slice(struct mjs* mjs) {
    size_t nargs = mjs_nargs(mjs);
    mjs_val_t src = mjs_get_this(mjs);

    size_t start = 0;
    size_t end = 0;
    char* src_buf = NULL;
    size_t src_len = 0;

    bool args_correct = false;
    do {
        if(!mjs_is_array_buf(src)) {
            break;
        }
        src_buf = mjs_array_buf_get_ptr(mjs, src, &src_len);

        if((nargs == 0) || (nargs > 2)) {
            break;
        }

        mjs_val_t start_obj = mjs_arg(mjs, 0);
        if(!mjs_is_number(start_obj)) {
            break;
        }
        start = mjs_get_int32(mjs, start_obj);

        if(nargs == 2) {
            mjs_val_t end_obj = mjs_arg(mjs, 1);
            if(!mjs_is_number(end_obj)) {
                break;
            }
            end = mjs_get_int32(mjs, end_obj);
        } else {
            end = src_len - 1;
        }

        if((start >= src_len) || (end >= src_len) || (start >= end)) {
            break;
        }

        args_correct = true;
    } while(0);

    if(!args_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    src_buf += start;
    mjs_return(mjs, mjs_mk_array_buf(mjs, src_buf, end - start));
}

static mjs_val_t
    mjs_mk_dataview_from_buf(struct mjs* mjs, mjs_val_t buf, mjs_dataview_type_t type) {
    size_t len = 0;
    mjs_array_buf_get_ptr(mjs, buf, &len);
    if(len % mjs_dataview_get_element_len(type) != 0) {
        mjs_prepend_errorf(
            mjs, MJS_BAD_ARGS_ERROR, "Buffer len is not a multiple of element size");
        return MJS_UNDEFINED;
    }
    mjs_val_t view_obj = mjs_mk_object(mjs);
    mjs_set(mjs, view_obj, "_t", ~0, mjs_mk_number(mjs, (double)type));
    mjs_set(mjs, view_obj, "buffer", ~0, buf);

    view_obj &= ~MJS_TAG_MASK;
    view_obj |= MJS_TAG_ARRAY_BUF_VIEW;

    mjs_dataview_get(mjs, view_obj, 0);

    return view_obj;
}

static mjs_val_t
    mjs_mk_dataview(struct mjs* mjs, size_t len, mjs_val_t arr, mjs_dataview_type_t type) {
    size_t elements_nb = 0;
    if(mjs_is_array(arr)) {
        if(!mjs_is_number(mjs_array_get(mjs, arr, 0))) {
            return MJS_UNDEFINED;
        }
        elements_nb = mjs_array_length(mjs, arr);
    } else {
        elements_nb = len;
    }

    size_t element_len = mjs_dataview_get_element_len(type);
    mjs_val_t buf_obj = mjs_mk_array_buf(mjs, NULL, element_len * elements_nb);

    if(mjs_is_array(arr)) {
        char* buf_ptr = mjs_array_buf_get_ptr(mjs, buf_obj, NULL);
        for(uint8_t i = 0; i < elements_nb; i++) {
            int64_t value = mjs_get_double(mjs, mjs_array_get(mjs, arr, i));
            set_value(buf_ptr, value, type);
            buf_ptr += element_len;
        }
    }

    return mjs_mk_dataview_from_buf(mjs, buf_obj, type);
}

static void mjs_array_buf_new(struct mjs* mjs) {
    mjs_val_t len_arg = mjs_arg(mjs, 0);
    mjs_val_t buf_obj = MJS_UNDEFINED;
    if(!mjs_is_number(len_arg)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
    } else {
        int len = mjs_get_int(mjs, len_arg);
        buf_obj = mjs_mk_array_buf(mjs, NULL, len);
    }
    mjs_return(mjs, buf_obj);
}

static void mjs_dataview_new(struct mjs* mjs, mjs_dataview_type_t type) {
    mjs_val_t view_arg = mjs_arg(mjs, 0);
    mjs_val_t view_obj = MJS_UNDEFINED;

    if(mjs_is_array_buf(view_arg)) { // Create a view of existing ArrayBuf
        view_obj = mjs_mk_dataview_from_buf(mjs, view_arg, type);
    } else if(mjs_is_number(view_arg)) { // Create new typed array
        int len = mjs_get_int(mjs, view_arg);
        view_obj = mjs_mk_dataview(mjs, len, MJS_UNDEFINED, type);
    } else if(mjs_is_array(view_arg)) { // Create new typed array from array
        view_obj = mjs_mk_dataview(mjs, 0, view_arg, type);
    } else {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
    }

    mjs_return(mjs, view_obj);
}

static void mjs_new_u8_array(struct mjs* mjs) {
    mjs_dataview_new(mjs, MJS_DATAVIEW_U8);
}

static void mjs_new_i8_array(struct mjs* mjs) {
    mjs_dataview_new(mjs, MJS_DATAVIEW_I8);
}

static void mjs_new_u16_array(struct mjs* mjs) {
    mjs_dataview_new(mjs, MJS_DATAVIEW_U16);
}

static void mjs_new_i16_array(struct mjs* mjs) {
    mjs_dataview_new(mjs, MJS_DATAVIEW_I16);
}

static void mjs_new_u32_array(struct mjs* mjs) {
    mjs_dataview_new(mjs, MJS_DATAVIEW_U32);
}

static void mjs_new_i32_array(struct mjs* mjs) {
    mjs_dataview_new(mjs, MJS_DATAVIEW_I32);
}

void mjs_init_builtin_array_buf(struct mjs* mjs, mjs_val_t obj) {
    mjs_set(mjs, obj, "ArrayBuffer", ~0, MJS_MK_FN(mjs_array_buf_new));
    mjs_set(mjs, obj, "Uint8Array", ~0, MJS_MK_FN(mjs_new_u8_array));
    mjs_set(mjs, obj, "Int8Array", ~0, MJS_MK_FN(mjs_new_i8_array));
    mjs_set(mjs, obj, "Uint16Array", ~0, MJS_MK_FN(mjs_new_u16_array));
    mjs_set(mjs, obj, "Int16Array", ~0, MJS_MK_FN(mjs_new_i16_array));
    mjs_set(mjs, obj, "Uint32Array", ~0, MJS_MK_FN(mjs_new_u32_array));
    mjs_set(mjs, obj, "Int32Array", ~0, MJS_MK_FN(mjs_new_i32_array));
}
