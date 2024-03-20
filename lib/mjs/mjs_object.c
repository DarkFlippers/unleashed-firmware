/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#include "mjs_object.h"
#include "mjs_core.h"
#include "mjs_internal.h"
#include "mjs_primitive.h"
#include "mjs_string.h"
#include "mjs_util.h"

#include "common/mg_str.h"

MJS_PRIVATE mjs_val_t mjs_object_to_value(struct mjs_object* o) {
    if(o == NULL) {
        return MJS_NULL;
    } else {
        return mjs_legit_pointer_to_value(o) | MJS_TAG_OBJECT;
    }
}

MJS_PRIVATE struct mjs_object* get_object_struct(mjs_val_t v) {
    struct mjs_object* ret = NULL;
    if(mjs_is_null(v)) {
        ret = NULL;
    } else {
        assert(mjs_is_object_based(v));
        ret = (struct mjs_object*)get_ptr(v);
    }
    return ret;
}

mjs_val_t mjs_mk_object(struct mjs* mjs) {
    struct mjs_object* o = new_object(mjs);
    if(o == NULL) {
        return MJS_NULL;
    }
    (void)mjs;
    o->properties = NULL;
    return mjs_object_to_value(o);
}

int mjs_is_object(mjs_val_t v) {
    return (v & MJS_TAG_MASK) == MJS_TAG_OBJECT || (v & MJS_TAG_MASK) == MJS_TAG_ARRAY;
}

int mjs_is_object_based(mjs_val_t v) {
    return ((v & MJS_TAG_MASK) == MJS_TAG_OBJECT) || ((v & MJS_TAG_MASK) == MJS_TAG_ARRAY) ||
           ((v & MJS_TAG_MASK) == MJS_TAG_ARRAY_BUF_VIEW);
}

MJS_PRIVATE struct mjs_property*
    mjs_get_own_property(struct mjs* mjs, mjs_val_t obj, const char* name, size_t len) {
    struct mjs_property* p;
    struct mjs_object* o;

    if(!mjs_is_object_based(obj)) {
        return NULL;
    }

    o = get_object_struct(obj);

    if(len <= 5) {
        mjs_val_t ss = mjs_mk_string(mjs, name, len, 1);
        for(p = o->properties; p != NULL; p = p->next) {
            if(p->name == ss) return p;
        }
    } else {
        for(p = o->properties; p != NULL; p = p->next) {
            if(mjs_strcmp(mjs, &p->name, name, len) == 0) return p;
        }
        return p;
    }

    return NULL;
}

MJS_PRIVATE struct mjs_property*
    mjs_get_own_property_v(struct mjs* mjs, mjs_val_t obj, mjs_val_t key) {
    size_t n;
    char* s = NULL;
    int need_free = 0;
    struct mjs_property* p = NULL;
    mjs_err_t err = mjs_to_string(mjs, &key, &s, &n, &need_free);
    if(err == MJS_OK) {
        p = mjs_get_own_property(mjs, obj, s, n);
    }
    if(need_free) free(s);
    return p;
}

MJS_PRIVATE struct mjs_property*
    mjs_mk_property(struct mjs* mjs, mjs_val_t name, mjs_val_t value) {
    struct mjs_property* p = new_property(mjs);
    p->next = NULL;
    p->name = name;
    p->value = value;
    return p;
}

mjs_val_t mjs_get(struct mjs* mjs, mjs_val_t obj, const char* name, size_t name_len) {
    struct mjs_property* p;

    if(name_len == (size_t)~0) {
        name_len = strlen(name);
    }

    p = mjs_get_own_property(mjs, obj, name, name_len);
    if(p == NULL) {
        return MJS_UNDEFINED;
    } else {
        return p->value;
    }
}

mjs_val_t mjs_get_v(struct mjs* mjs, mjs_val_t obj, mjs_val_t name) {
    size_t n;
    char* s = NULL;
    int need_free = 0;
    mjs_val_t ret = MJS_UNDEFINED;

    mjs_err_t err = mjs_to_string(mjs, &name, &s, &n, &need_free);

    if(err == MJS_OK) {
        /* Successfully converted name value to string: get the property */
        ret = mjs_get(mjs, obj, s, n);
    }

    if(need_free) {
        free(s);
        s = NULL;
    }
    return ret;
}

mjs_val_t mjs_get_v_proto(struct mjs* mjs, mjs_val_t obj, mjs_val_t key) {
    struct mjs_property* p;
    mjs_val_t pn = mjs_mk_string(mjs, MJS_PROTO_PROP_NAME, ~0, 1);
    if((p = mjs_get_own_property_v(mjs, obj, key)) != NULL) return p->value;
    if((p = mjs_get_own_property_v(mjs, obj, pn)) == NULL) return MJS_UNDEFINED;
    return mjs_get_v_proto(mjs, p->value, key);
}

mjs_err_t
    mjs_set(struct mjs* mjs, mjs_val_t obj, const char* name, size_t name_len, mjs_val_t val) {
    return mjs_set_internal(mjs, obj, MJS_UNDEFINED, (char*)name, name_len, val);
}

mjs_err_t mjs_set_v(struct mjs* mjs, mjs_val_t obj, mjs_val_t name, mjs_val_t val) {
    return mjs_set_internal(mjs, obj, name, NULL, 0, val);
}

MJS_PRIVATE mjs_err_t mjs_set_internal(
    struct mjs* mjs,
    mjs_val_t obj,
    mjs_val_t name_v,
    char* name,
    size_t name_len,
    mjs_val_t val) {
    mjs_err_t rcode = MJS_OK;

    struct mjs_property* p;

    int need_free = 0;

    if(name == NULL) {
        /* Pointer was not provided, so obtain one from the name_v. */
        rcode = mjs_to_string(mjs, &name_v, &name, &name_len, &need_free);
        if(rcode != MJS_OK) {
            goto clean;
        }
    } else {
        /*
     * Pointer was provided, so we ignore name_v. Here we set it to undefined,
     * and the actual value will be calculated later if needed.
     */
        name_v = MJS_UNDEFINED;
    }

    p = mjs_get_own_property(mjs, obj, name, name_len);

    if(p == NULL) {
        struct mjs_object* o;
        if(!mjs_is_object_based(obj)) {
            return MJS_REFERENCE_ERROR;
        }

        /*
     * name_v might be not a string here. In this case, we need to create a new
     * `name_v`, which will be a string.
     */
        if(!mjs_is_string(name_v)) {
            name_v = mjs_mk_string(mjs, name, name_len, 1);
        }

        p = mjs_mk_property(mjs, name_v, val);

        o = get_object_struct(obj);
        p->next = o->properties;
        o->properties = p;
    }

    p->value = val;

clean:
    if(need_free) {
        free(name);
        name = NULL;
    }
    return rcode;
}

MJS_PRIVATE void mjs_destroy_property(struct mjs_property** p) {
    *p = NULL;
}

/*
 * See comments in `object_public.h`
 */
int mjs_del(struct mjs* mjs, mjs_val_t obj, const char* name, size_t len) {
    struct mjs_property *prop, *prev;

    if(!mjs_is_object_based(obj)) {
        return -1;
    }
    if(len == (size_t)~0) {
        len = strlen(name);
    }
    for(prev = NULL, prop = get_object_struct(obj)->properties; prop != NULL;
        prev = prop, prop = prop->next) {
        size_t n;
        const char* s = mjs_get_string(mjs, &prop->name, &n);
        if(n == len && strncmp(s, name, len) == 0) {
            if(prev) {
                prev->next = prop->next;
            } else {
                get_object_struct(obj)->properties = prop->next;
            }
            mjs_destroy_property(&prop);
            return 0;
        }
    }
    return -1;
}

mjs_val_t mjs_next(struct mjs* mjs, mjs_val_t obj, mjs_val_t* iterator) {
    struct mjs_property* p = NULL;
    mjs_val_t key = MJS_UNDEFINED;

    if(*iterator == MJS_UNDEFINED) {
        struct mjs_object* o = get_object_struct(obj);
        p = o->properties;
    } else {
        p = ((struct mjs_property*)get_ptr(*iterator))->next;
    }

    if(p == NULL) {
        *iterator = MJS_UNDEFINED;
    } else {
        key = p->name;
        *iterator = mjs_mk_foreign(mjs, p);
    }

    return key;
}

MJS_PRIVATE void mjs_op_create_object(struct mjs* mjs) {
    mjs_val_t ret = MJS_UNDEFINED;
    mjs_val_t proto_v = mjs_arg(mjs, 0);

    if(!mjs_check_arg(mjs, 0, "proto", MJS_TYPE_OBJECT_GENERIC, &proto_v)) {
        goto clean;
    }

    ret = mjs_mk_object(mjs);
    mjs_set(mjs, ret, MJS_PROTO_PROP_NAME, ~0, proto_v);

clean:
    mjs_return(mjs, ret);
}

mjs_val_t
    mjs_struct_to_obj(struct mjs* mjs, const void* base, const struct mjs_c_struct_member* defs) {
    mjs_val_t obj;
    const struct mjs_c_struct_member* def = defs;
    if(base == NULL || def == NULL) return MJS_UNDEFINED;
    obj = mjs_mk_object(mjs);
    /* Pin the object while it is being built */
    mjs_own(mjs, &obj);
    /*
   * Because mjs inserts new properties at the head of the list,
   * start from the end so the constructed object more closely resembles
   * the definition.
   */
    while(def->name != NULL) def++;
    for(def--; def >= defs; def--) {
        mjs_val_t v = MJS_UNDEFINED;
        const char* ptr = (const char*)base + def->offset;
        switch(def->type) {
        case MJS_STRUCT_FIELD_TYPE_STRUCT: {
            const void* sub_base = (const void*)ptr;
            const struct mjs_c_struct_member* sub_def =
                (const struct mjs_c_struct_member*)def->arg;
            v = mjs_struct_to_obj(mjs, sub_base, sub_def);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_STRUCT_PTR: {
            const void** sub_base = (const void**)ptr;
            const struct mjs_c_struct_member* sub_def =
                (const struct mjs_c_struct_member*)def->arg;
            if(*sub_base != NULL) {
                v = mjs_struct_to_obj(mjs, *sub_base, sub_def);
            } else {
                v = MJS_NULL;
            }
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_INT: {
            double value = (double)(*(int*)ptr);
            v = mjs_mk_number(mjs, value);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_BOOL: {
            v = mjs_mk_boolean(mjs, *(bool*)ptr);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_DOUBLE: {
            v = mjs_mk_number(mjs, *(double*)ptr);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_FLOAT: {
            float value = *(float*)ptr;
            v = mjs_mk_number(mjs, value);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_CHAR_PTR: {
            const char* value = *(const char**)ptr;
            v = mjs_mk_string(mjs, value, ~0, 1);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_VOID_PTR: {
            v = mjs_mk_foreign(mjs, *(void**)ptr);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_MG_STR_PTR: {
            const struct mg_str* s = *(const struct mg_str**)ptr;
            if(s != NULL) {
                v = mjs_mk_string(mjs, s->p, s->len, 1);
            } else {
                v = MJS_NULL;
            }
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_MG_STR: {
            const struct mg_str* s = (const struct mg_str*)ptr;
            v = mjs_mk_string(mjs, s->p, s->len, 1);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_DATA: {
            const char* dptr = (const char*)ptr;
            const intptr_t dlen = (intptr_t)def->arg;
            v = mjs_mk_string(mjs, dptr, dlen, 1);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_INT8: {
            double value = (double)(*(int8_t*)ptr);
            v = mjs_mk_number(mjs, value);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_INT16: {
            double value = (double)(*(int16_t*)ptr);
            v = mjs_mk_number(mjs, value);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_UINT8: {
            double value = (double)(*(uint8_t*)ptr);
            v = mjs_mk_number(mjs, value);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_UINT16: {
            double value = (double)(*(uint16_t*)ptr);
            v = mjs_mk_number(mjs, value);
            break;
        }
        case MJS_STRUCT_FIELD_TYPE_CUSTOM: {
            mjs_val_t (*fptr)(struct mjs*, const void*) =
                (mjs_val_t(*)(struct mjs*, const void*))def->arg;
            v = fptr(mjs, ptr);
        }
        default: {
            break;
        }
        }
        mjs_set(mjs, obj, def->name, ~0, v);
    }
    mjs_disown(mjs, &obj);
    return obj;
}
