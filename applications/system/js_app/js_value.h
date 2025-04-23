#pragma once

#include <furi.h>
#include "js_modules.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    // literal types
    JsValueTypeAny, //<! Literal term
    JsValueTypeAnyArray, //<! Literal term, after ensuring that it's an array
    JsValueTypeAnyObject, //<! Literal term, after ensuring that it's an object
    JsValueTypeFunction, //<! Literal term, after ensuring that it's a function

    // primitive types
    JsValueTypeRawPointer, //<! Unchecked `void*`
    JsValueTypeInt32, //<! Number cast to `int32_t`
    JsValueTypeDouble, //<! Number cast to `double`
    JsValueTypeString, //<! Any string cast to `const char*`
    JsValueTypeBool, //<! Bool cast to `bool`

    // types with children
    JsValueTypeEnum, //<! String with predefined possible values cast to a C enum via a mapping
    JsValueTypeObject, //<! Object with predefined recursive fields cast to several C values

    JsValueTypeMask = 0xff,

    // enum sizes
    JsValueTypeEnumSize1 = (1 << 8),
    JsValueTypeEnumSize2 = (2 << 8),
    JsValueTypeEnumSize4 = (4 << 8),

    // flags
    JsValueTypePermitNull = (1 << 16), //<! If the value is absent, assign default value
} JsValueType;

#define JS_VALUE_TYPE_ENUM_SIZE(x) ((x) << 8)

typedef struct {
    const char* string_value;
    size_t num_value;
} JsValueEnumVariant;

typedef union {
    void* ptr_val;
    int32_t int32_val;
    double double_val;
    const char* str_val;
    size_t enum_val;
    bool bool_val;
} JsValueDefaultValue;

typedef struct JsValueObjectField JsValueObjectField;

typedef struct {
    JsValueType type;
    JsValueDefaultValue default_value;

    size_t n_children;
    union {
        const JsValueEnumVariant* enum_variants;
        const JsValueObjectField* object_fields;
    };
} JsValueDeclaration;

struct JsValueObjectField {
    const char* field_name;
    const JsValueDeclaration* value;
};

typedef struct {
    size_t n_children;
    const JsValueDeclaration* arguments;
} JsValueArguments;

#define JS_VALUE_ENUM(c_type, variants)                                    \
    {                                                                      \
        .type = JsValueTypeEnum | JS_VALUE_TYPE_ENUM_SIZE(sizeof(c_type)), \
        .n_children = COUNT_OF(variants),                                  \
        .enum_variants = variants,                                         \
    }

#define JS_VALUE_ENUM_W_DEFAULT(c_type, variants, default) \
    {                                                      \
        .type = JsValueTypeEnum | JsValueTypePermitNull |  \
                JS_VALUE_TYPE_ENUM_SIZE(sizeof(c_type)),   \
        .default_value.enum_val = default,                 \
        .n_children = COUNT_OF(variants),                  \
        .enum_variants = variants,                         \
    }

#define JS_VALUE_OBJECT(fields)         \
    {                                   \
        .type = JsValueTypeObject,      \
        .n_children = COUNT_OF(fields), \
        .object_fields = fields,        \
    }

#define JS_VALUE_OBJECT_W_DEFAULTS(fields)                 \
    {                                                      \
        .type = JsValueTypeObject | JsValueTypePermitNull, \
        .n_children = COUNT_OF(fields),                    \
        .object_fields = fields,                           \
    }

#define JS_VALUE_SIMPLE(t) {.type = t}

#define JS_VALUE_SIMPLE_W_DEFAULT(t, name, val) \
    {.type = (t) | JsValueTypePermitNull, .default_value.name = (val)}

#define JS_VALUE_ARGS(args)           \
    {                                 \
        .n_children = COUNT_OF(args), \
        .arguments = args,            \
    }

typedef enum {
    JsValueParseFlagNone = 0,
    JsValueParseFlagReturnOnError =
        (1
         << 0), //<! Sets mjs error string to a description of the parsing error and returns from the JS function
} JsValueParseFlag;

typedef enum {
    JsValueParseStatusOk, //<! Parsing completed successfully
    JsValueParseStatusJsError, //<! Parsing failed due to incorrect JS input
} JsValueParseStatus;

typedef enum {
    JsValueParseSourceValue,
    JsValueParseSourceArguments,
} JsValueParseSource;

typedef struct {
    JsValueParseSource source;
    union {
        const JsValueDeclaration* value_decl;
        const JsValueArguments* argument_decl;
    };
} JsValueParseDeclaration;

#define JS_VALUE_PARSE_SOURCE_VALUE(declaration) \
    ((JsValueParseDeclaration){.source = JsValueParseSourceValue, .value_decl = declaration})
#define JS_VALUE_PARSE_SOURCE_ARGS(declaration) \
    ((JsValueParseDeclaration){                 \
        .source = JsValueParseSourceArguments, .argument_decl = declaration})

/**
 * @brief Determines the size of the buffer array of `mjs_val_t`s that needs to
 * be passed to `js_value_parse`.
 */
size_t js_value_buffer_size(const JsValueParseDeclaration declaration);

/**
 * @brief Converts a JS value into a series of C values.
 * 
 * @param[in]    mjs         mJS instance pointer
 * @param[in]    declaration Declaration for the input value. Chooses where the
 *                           values are to be fetched from (an `mjs_val_t` or
 *                           function arguments)
 * @param[in]    flags       See the corresponding enum.
 * @param[out]   buffer      Temporary buffer for values that need to live
 *                           longer than the function call. To determine the
 *                           size of the buffer, use `js_value_buffer_size`.
 *                           Values parsed by this function will become invalid
 *                           when this buffer goes out of scope.
 * @param[in]    buf_size    Number of entries in the temporary buffer (i.e.
 *                           `COUNT_OF`, not `sizeof`).
 * @param[in]    source      Source JS value that needs to be converted. May be
 *                           NULL if `declaration.source` is
 *                           `JsValueParseSourceArguments`.
 * @param[in]    n_c_vals    Number of output C values
 * @param[out]   ...         Pointers to output C values. The order in which
 *                           these values are populated corresponds to the order
 *                           in which the values are defined in the declaration.
 * 
 * @returns Parsing status
 */
JsValueParseStatus js_value_parse(
    struct mjs* mjs,
    const JsValueParseDeclaration declaration,
    JsValueParseFlag flags,
    mjs_val_t* buffer,
    size_t buf_size,
    mjs_val_t* source,
    size_t n_c_vals,
    ...);

#define JS_VALUE_PARSE(mjs, declaration, flags, status_ptr, value_ptr, ...) \
    void* _args[] = {__VA_ARGS__};                                          \
    size_t _n_args = COUNT_OF(_args);                                       \
    size_t _temp_buf_len = js_value_buffer_size(declaration);               \
    mjs_val_t _temp_buffer[_temp_buf_len];                                  \
    *(status_ptr) = js_value_parse(                                         \
        mjs, declaration, flags, _temp_buffer, _temp_buf_len, value_ptr, _n_args, __VA_ARGS__);

#define JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, declaration, ...) \
    JsValueParseStatus _status;                              \
    JS_VALUE_PARSE(                                          \
        mjs,                                                 \
        JS_VALUE_PARSE_SOURCE_ARGS(declaration),             \
        JsValueParseFlagReturnOnError,                       \
        &_status,                                            \
        NULL,                                                \
        __VA_ARGS__);                                        \
    if(_status != JsValueParseStatusOk) return;

#ifdef __cplusplus
}
#endif
