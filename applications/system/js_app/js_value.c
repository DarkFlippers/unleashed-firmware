#include "js_value.h"
#include <stdarg.h>

#ifdef APP_UNIT_TESTS
#define JS_VAL_DEBUG
#endif

size_t js_value_buffer_size(const JsValueParseDeclaration declaration) {
    if(declaration.source == JsValueParseSourceValue) {
        const JsValueDeclaration* value_decl = declaration.value_decl;
        JsValueType type = value_decl->type & JsValueTypeMask;

        if(type == JsValueTypeString) return 1;

        if(type == JsValueTypeObject) {
            size_t total = 0;
            for(size_t i = 0; i < value_decl->n_children; i++)
                total += js_value_buffer_size(
                    JS_VALUE_PARSE_SOURCE_VALUE(value_decl->object_fields[i].value));
            return total;
        }

        return 0;

    } else {
        const JsValueArguments* arg_decl = declaration.argument_decl;
        size_t total = 0;
        for(size_t i = 0; i < arg_decl->n_children; i++)
            total += js_value_buffer_size(JS_VALUE_PARSE_SOURCE_VALUE(&arg_decl->arguments[i]));
        return total;
    }
}

static size_t js_value_resulting_c_values_count(const JsValueParseDeclaration declaration) {
    if(declaration.source == JsValueParseSourceValue) {
        const JsValueDeclaration* value_decl = declaration.value_decl;
        JsValueType type = value_decl->type & JsValueTypeMask;

        if(type == JsValueTypeObject) {
            size_t total = 0;
            for(size_t i = 0; i < value_decl->n_children; i++)
                total += js_value_resulting_c_values_count(
                    JS_VALUE_PARSE_SOURCE_VALUE(value_decl->object_fields[i].value));
            return total;
        }

        return 1;

    } else {
        const JsValueArguments* arg_decl = declaration.argument_decl;
        size_t total = 0;
        for(size_t i = 0; i < arg_decl->n_children; i++)
            total += js_value_resulting_c_values_count(
                JS_VALUE_PARSE_SOURCE_VALUE(&arg_decl->arguments[i]));
        return total;
    }
}

#define PREPEND_JS_ERROR_AND_RETURN(mjs, flags, ...)                    \
    do {                                                                \
        if((flags) & JsValueParseFlagReturnOnError)                     \
            mjs_prepend_errorf((mjs), MJS_BAD_ARGS_ERROR, __VA_ARGS__); \
        return JsValueParseStatusJsError;                               \
    } while(0)

#define PREPEND_JS_EXPECTED_ERROR_AND_RETURN(mjs, flags, type) \
    PREPEND_JS_ERROR_AND_RETURN(mjs, flags, "expected %s", type)

static void js_value_assign_enum_val(void* destination, JsValueType type_w_flags, uint32_t value) {
    if(type_w_flags & JsValueTypeEnumSize1) {
        *(uint8_t*)destination = value;
    } else if(type_w_flags & JsValueTypeEnumSize2) {
        *(uint16_t*)destination = value;
    } else if(type_w_flags & JsValueTypeEnumSize4) {
        *(uint32_t*)destination = value;
    }
}

static bool js_value_is_null_or_undefined(mjs_val_t* val_ptr) {
    return mjs_is_null(*val_ptr) || mjs_is_undefined(*val_ptr);
}

static bool js_value_maybe_assign_default(
    const JsValueDeclaration* declaration,
    mjs_val_t* val_ptr,
    void* destination,
    size_t size) {
    if((declaration->type & JsValueTypePermitNull) && js_value_is_null_or_undefined(val_ptr)) {
        memcpy(destination, &declaration->default_value, size);
        return true;
    }
    return false;
}

typedef int (*MjsTypecheckFn)(mjs_val_t value);

static JsValueParseStatus js_value_parse_literal(
    struct mjs* mjs,
    JsValueParseFlag flags,
    mjs_val_t* destination,
    mjs_val_t* source,
    MjsTypecheckFn typecheck,
    const char* type_name) {
    if(!typecheck(*source)) PREPEND_JS_EXPECTED_ERROR_AND_RETURN(mjs, flags, type_name);
    *destination = *source;
    return JsValueParseStatusOk;
}

static JsValueParseStatus js_value_parse_va(
    struct mjs* mjs,
    const JsValueParseDeclaration declaration,
    JsValueParseFlag flags,
    mjs_val_t* source,
    mjs_val_t* buffer,
    size_t* buffer_index,
    va_list* out_pointers) {
    if(declaration.source == JsValueParseSourceArguments) {
        const JsValueArguments* arg_decl = declaration.argument_decl;

        for(size_t i = 0; i < arg_decl->n_children; i++) {
            mjs_val_t arg_val = mjs_arg(mjs, i);
            JsValueParseStatus status = js_value_parse_va(
                mjs,
                JS_VALUE_PARSE_SOURCE_VALUE(&arg_decl->arguments[i]),
                flags,
                &arg_val,
                buffer,
                buffer_index,
                out_pointers);
            if(status != JsValueParseStatusOk) return status;
        }

        return JsValueParseStatusOk;
    }

    const JsValueDeclaration* value_decl = declaration.value_decl;
    JsValueType type_w_flags = value_decl->type;
    JsValueType type_noflags = type_w_flags & JsValueTypeMask;
    bool is_null_but_allowed = (type_w_flags & JsValueTypePermitNull) &&
                               js_value_is_null_or_undefined(source);

    void* destination = NULL;
    if(type_noflags != JsValueTypeObject) destination = va_arg(*out_pointers, void*);

    switch(type_noflags) {
    // Literal terms
    case JsValueTypeAny:
        *(mjs_val_t*)destination = *source;
        break;
    case JsValueTypeAnyArray:
        return js_value_parse_literal(mjs, flags, destination, source, mjs_is_array, "array");
    case JsValueTypeAnyObject:
        return js_value_parse_literal(mjs, flags, destination, source, mjs_is_object, "array");
    case JsValueTypeFunction:
        return js_value_parse_literal(
            mjs, flags, destination, source, mjs_is_function, "function");

    // Primitive types
    case JsValueTypeRawPointer: {
        if(js_value_maybe_assign_default(value_decl, source, destination, sizeof(void*))) break;
        if(!mjs_is_foreign(*source)) PREPEND_JS_EXPECTED_ERROR_AND_RETURN(mjs, flags, "pointer");
        *(void**)destination = mjs_get_ptr(mjs, *source);
        break;
    }
    case JsValueTypeInt32: {
        if(js_value_maybe_assign_default(value_decl, source, destination, sizeof(int32_t))) break;
        if(!mjs_is_number(*source)) PREPEND_JS_EXPECTED_ERROR_AND_RETURN(mjs, flags, "number");
        *(int32_t*)destination = mjs_get_int32(mjs, *source);
        break;
    }
    case JsValueTypeDouble: {
        if(js_value_maybe_assign_default(value_decl, source, destination, sizeof(double))) break;
        if(!mjs_is_number(*source)) PREPEND_JS_EXPECTED_ERROR_AND_RETURN(mjs, flags, "number");
        *(double*)destination = mjs_get_double(mjs, *source);
        break;
    }
    case JsValueTypeBool: {
        if(js_value_maybe_assign_default(value_decl, source, destination, sizeof(bool))) break;
        if(!mjs_is_boolean(*source)) PREPEND_JS_EXPECTED_ERROR_AND_RETURN(mjs, flags, "bool");
        *(bool*)destination = mjs_get_bool(mjs, *source);
        break;
    }
    case JsValueTypeString: {
        if(js_value_maybe_assign_default(value_decl, source, destination, sizeof(const char*)))
            break;
        if(!mjs_is_string(*source)) PREPEND_JS_EXPECTED_ERROR_AND_RETURN(mjs, flags, "string");
        buffer[*buffer_index] = *source;
        *(const char**)destination = mjs_get_string(mjs, &buffer[*buffer_index], NULL);
        (*buffer_index)++;
        break;
    }

    // Types with children
    case JsValueTypeEnum: {
        if(is_null_but_allowed) {
            js_value_assign_enum_val(
                destination, type_w_flags, value_decl->default_value.enum_val);

        } else if(mjs_is_string(*source)) {
            const char* str = mjs_get_string(mjs, source, NULL);
            furi_check(str);

            bool match_found = false;
            for(size_t i = 0; i < value_decl->n_children; i++) {
                const JsValueEnumVariant* variant = &value_decl->enum_variants[i];
                if(strcmp(str, variant->string_value) == 0) {
                    js_value_assign_enum_val(destination, type_w_flags, variant->num_value);
                    match_found = true;
                    break;
                }
            }

            if(!match_found)
                PREPEND_JS_EXPECTED_ERROR_AND_RETURN(mjs, flags, "one of permitted strings");

        } else {
            PREPEND_JS_EXPECTED_ERROR_AND_RETURN(mjs, flags, "string");
        }
        break;
    }

    case JsValueTypeObject: {
        if(!(is_null_but_allowed || mjs_is_object(*source)))
            PREPEND_JS_EXPECTED_ERROR_AND_RETURN(mjs, flags, "object");
        for(size_t i = 0; i < value_decl->n_children; i++) {
            const JsValueObjectField* field = &value_decl->object_fields[i];
            mjs_val_t field_val = mjs_get(mjs, *source, field->field_name, ~0);
            JsValueParseStatus status = js_value_parse_va(
                mjs,
                JS_VALUE_PARSE_SOURCE_VALUE(field->value),
                flags,
                &field_val,
                buffer,
                buffer_index,
                out_pointers);
            if(status != JsValueParseStatusOk)
                PREPEND_JS_ERROR_AND_RETURN(mjs, flags, "field %s: ", field->field_name);
        }
        break;
    }

    case JsValueTypeMask:
    case JsValueTypeEnumSize1:
    case JsValueTypeEnumSize2:
    case JsValueTypeEnumSize4:
    case JsValueTypePermitNull:
        furi_crash();
    }

    return JsValueParseStatusOk;
}

JsValueParseStatus js_value_parse(
    struct mjs* mjs,
    const JsValueParseDeclaration declaration,
    JsValueParseFlag flags,
    mjs_val_t* buffer,
    size_t buf_size,
    mjs_val_t* source,
    size_t n_c_vals,
    ...) {
    furi_check(mjs);
    furi_check(buffer);

    if(declaration.source == JsValueParseSourceValue) {
        furi_check(source);
        furi_check(declaration.value_decl);
    } else {
        furi_check(source == NULL);
        furi_check(declaration.argument_decl);
    }

#ifdef JS_VAL_DEBUG
    furi_check(buf_size == js_value_buffer_size(declaration));
    furi_check(n_c_vals == js_value_resulting_c_values_count(declaration));
#else
    UNUSED(js_value_resulting_c_values_count);
#endif

    va_list out_pointers;
    va_start(out_pointers, n_c_vals);

    size_t buffer_index = 0;
    JsValueParseStatus status =
        js_value_parse_va(mjs, declaration, flags, source, buffer, &buffer_index, &out_pointers);
    furi_check(buffer_index <= buf_size);

    va_end(out_pointers);

    return status;
}
