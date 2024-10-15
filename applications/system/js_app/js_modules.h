#pragma once

#include <stdint.h>
#include "js_thread_i.h"
#include <flipper_application/flipper_application.h>
#include <flipper_application/plugins/plugin_manager.h>
#include <flipper_application/plugins/composite_resolver.h>

#define PLUGIN_APP_ID      "js"
#define PLUGIN_API_VERSION 1

/**
 * @brief Returns the foreign pointer in `obj["_"]`
 */
#define JS_GET_INST(mjs, obj) mjs_get_ptr(mjs, mjs_get(mjs, obj, INST_PROP_NAME, ~0))
/**
 * @brief Returns the foreign pointer in `this["_"]`
 */
#define JS_GET_CONTEXT(mjs)   JS_GET_INST(mjs, mjs_get_this(mjs))

/**
 * @brief Syntax sugar for constructing an object
 * 
 * @example
 * ```c
 *  mjs_val_t my_obj = mjs_mk_object(mjs);
 *  JS_ASSIGN_MULTI(mjs, my_obj) {
 *      JS_FIELD("method1", MJS_MK_FN(js_storage_file_is_open));
 *      JS_FIELD("method2", MJS_MK_FN(js_storage_file_is_open));
 *  }
 * ```
 */
#define JS_ASSIGN_MULTI(mjs, object)     \
    for(struct {                         \
            struct mjs* mjs;             \
            mjs_val_t val;               \
            int i;                       \
        } _ass_multi = {mjs, object, 0}; \
        _ass_multi.i == 0;               \
        _ass_multi.i++)
#define JS_FIELD(name, value) mjs_set(_ass_multi.mjs, _ass_multi.val, name, ~0, value)

/**
 * @brief The first word of structures that foreign pointer JS values point to
 * 
 * This is used to detect situations where JS code mistakenly passes an opaque
 * foreign pointer of one type as an argument to a native function which expects
 * a struct of another type.
 * 
 * It is recommended to use this functionality in conjunction with the following
 * convenience verification macros:
 *   - `JS_ARG_STRUCT()`
 *   - `JS_ARG_OBJ_WITH_STRUCT()`
 * 
 * @warning In order for the mechanism to work properly, your struct must store
 * the magic value in the first word.
 */
typedef enum {
    JsForeignMagicStart = 0x15BAD000,
    JsForeignMagic_JsEventLoopContract,
} JsForeignMagic;

// Are you tired of your silly little JS+C glue code functions being 75%
// argument validation code and 25% actual logic? Introducing: ASS (Argument
// Schema for Scripts)! ASS is a set of macros that reduce the typical
// boilerplate code of "check argument count, get arguments, validate arguments,
// extract C values from arguments" down to just one line!

/**
 * When passed as the second argument to `JS_FETCH_ARGS_OR_RETURN`, signifies
 * that the function requires exactly as many arguments as were specified.
 */
#define JS_EXACTLY  ==
/**
 * When passed as the second argument to `JS_FETCH_ARGS_OR_RETURN`, signifies
 * that the function requires at least as many arguments as were specified.
 */
#define JS_AT_LEAST >=

#define JS_ENUM_MAP(var_name, ...)                      \
    static const JsEnumMapping var_name##_mapping[] = { \
        {NULL, sizeof(var_name)},                       \
        __VA_ARGS__,                                    \
        {NULL, 0},                                      \
    };

typedef struct {
    const char* name;
    size_t value;
} JsEnumMapping;

typedef struct {
    void* out;
    int (*validator)(mjs_val_t);
    void (*converter)(struct mjs*, mjs_val_t*, void* out, const void* extra);
    const char* expected_type;
    bool (*extended_validator)(struct mjs*, mjs_val_t, const void* extra);
    const void* extra_data;
} _js_arg_decl;

static inline void _js_to_int32(struct mjs* mjs, mjs_val_t* in, void* out, const void* extra) {
    UNUSED(extra);
    *(int32_t*)out = mjs_get_int32(mjs, *in);
}
#define JS_ARG_INT32(out) ((_js_arg_decl){out, mjs_is_number, _js_to_int32, "number", NULL, NULL})

static inline void _js_to_ptr(struct mjs* mjs, mjs_val_t* in, void* out, const void* extra) {
    UNUSED(extra);
    *(void**)out = mjs_get_ptr(mjs, *in);
}
#define JS_ARG_PTR(out) \
    ((_js_arg_decl){out, mjs_is_foreign, _js_to_ptr, "opaque pointer", NULL, NULL})

static inline void _js_to_string(struct mjs* mjs, mjs_val_t* in, void* out, const void* extra) {
    UNUSED(extra);
    *(const char**)out = mjs_get_string(mjs, in, NULL);
}
#define JS_ARG_STR(out) ((_js_arg_decl){out, mjs_is_string, _js_to_string, "string", NULL, NULL})

static inline void _js_to_bool(struct mjs* mjs, mjs_val_t* in, void* out, const void* extra) {
    UNUSED(extra);
    *(bool*)out = !!mjs_get_bool(mjs, *in);
}
#define JS_ARG_BOOL(out) ((_js_arg_decl){out, mjs_is_boolean, _js_to_bool, "boolean", NULL, NULL})

static inline void _js_passthrough(struct mjs* mjs, mjs_val_t* in, void* out, const void* extra) {
    UNUSED(extra);
    UNUSED(mjs);
    *(mjs_val_t*)out = *in;
}
#define JS_ARG_ANY(out) ((_js_arg_decl){out, NULL, _js_passthrough, "any", NULL, NULL})
#define JS_ARG_OBJ(out) ((_js_arg_decl){out, mjs_is_object, _js_passthrough, "any", NULL, NULL})
#define JS_ARG_FN(out) \
    ((_js_arg_decl){out, mjs_is_function, _js_passthrough, "function", NULL, NULL})
#define JS_ARG_ARR(out) ((_js_arg_decl){out, mjs_is_array, _js_passthrough, "array", NULL, NULL})

static inline bool _js_validate_struct(struct mjs* mjs, mjs_val_t val, const void* extra) {
    JsForeignMagic expected_magic = (JsForeignMagic)(size_t)extra;
    JsForeignMagic struct_magic = *(JsForeignMagic*)mjs_get_ptr(mjs, val);
    return struct_magic == expected_magic;
}
#define JS_ARG_STRUCT(type, out) \
    ((_js_arg_decl){             \
        out,                     \
        mjs_is_foreign,          \
        _js_to_ptr,              \
        #type,                   \
        _js_validate_struct,     \
        (void*)JsForeignMagic##_##type})

static inline bool _js_validate_obj_w_struct(struct mjs* mjs, mjs_val_t val, const void* extra) {
    JsForeignMagic expected_magic = (JsForeignMagic)(size_t)extra;
    JsForeignMagic struct_magic = *(JsForeignMagic*)JS_GET_INST(mjs, val);
    return struct_magic == expected_magic;
}
#define JS_ARG_OBJ_WITH_STRUCT(type, out) \
    ((_js_arg_decl){                      \
        out,                              \
        mjs_is_object,                    \
        _js_passthrough,                  \
        #type,                            \
        _js_validate_obj_w_struct,        \
        (void*)JsForeignMagic##_##type})

static inline bool _js_validate_enum(struct mjs* mjs, mjs_val_t val, const void* extra) {
    for(const JsEnumMapping* mapping = (JsEnumMapping*)extra + 1; mapping->name; mapping++)
        if(strcmp(mapping->name, mjs_get_string(mjs, &val, NULL)) == 0) return true;
    return false;
}
static inline void
    _js_convert_enum(struct mjs* mjs, mjs_val_t* val, void* out, const void* extra) {
    const JsEnumMapping* mapping = (JsEnumMapping*)extra;
    size_t size = mapping->value; // get enum size from first entry
    for(mapping++; mapping->name; mapping++) {
        if(strcmp(mapping->name, mjs_get_string(mjs, val, NULL)) == 0) {
            if(size == 1)
                *(uint8_t*)out = mapping->value;
            else if(size == 2)
                *(uint16_t*)out = mapping->value;
            else if(size == 4)
                *(uint32_t*)out = mapping->value;
            else if(size == 8)
                *(uint64_t*)out = mapping->value;
            return;
        }
    }
    // unreachable, thanks to _js_validate_enum
}
#define JS_ARG_ENUM(var_name, name) \
    ((_js_arg_decl){                \
        &var_name,                  \
        mjs_is_string,              \
        _js_convert_enum,           \
        name " enum",               \
        _js_validate_enum,          \
        var_name##_mapping})

//-V:JS_FETCH_ARGS_OR_RETURN:1008
/**
 * @brief Fetches and validates the arguments passed to a JS function
 * 
 * Example: `int32_t my_arg; JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_INT32(&my_arg));`
 * 
 * @warning This macro executes `return;` by design in case of an argument count
 * mismatch or a validation failure
 */
#define JS_FETCH_ARGS_OR_RETURN(mjs, arg_operator, ...)                                          \
    _js_arg_decl _js_args[] = {__VA_ARGS__};                                                     \
    int _js_arg_cnt = COUNT_OF(_js_args);                                                        \
    mjs_val_t _js_arg_vals[_js_arg_cnt];                                                         \
    if(!(mjs_nargs(mjs) arg_operator _js_arg_cnt))                                               \
        JS_ERROR_AND_RETURN(                                                                     \
            mjs,                                                                                 \
            MJS_BAD_ARGS_ERROR,                                                                  \
            "expected %s%d arguments, got %d",                                                   \
            #arg_operator,                                                                       \
            _js_arg_cnt,                                                                         \
            mjs_nargs(mjs));                                                                     \
    for(int _i = 0; _i < _js_arg_cnt; _i++) {                                                    \
        _js_arg_vals[_i] = mjs_arg(mjs, _i);                                                     \
        if(_js_args[_i].validator)                                                               \
            if(!_js_args[_i].validator(_js_arg_vals[_i]))                                        \
                JS_ERROR_AND_RETURN(                                                             \
                    mjs,                                                                         \
                    MJS_BAD_ARGS_ERROR,                                                          \
                    "argument %d: expected %s",                                                  \
                    _i,                                                                          \
                    _js_args[_i].expected_type);                                                 \
        if(_js_args[_i].extended_validator)                                                      \
            if(!_js_args[_i].extended_validator(mjs, _js_arg_vals[_i], _js_args[_i].extra_data)) \
                JS_ERROR_AND_RETURN(                                                             \
                    mjs,                                                                         \
                    MJS_BAD_ARGS_ERROR,                                                          \
                    "argument %d: expected %s",                                                  \
                    _i,                                                                          \
                    _js_args[_i].expected_type);                                                 \
        _js_args[_i].converter(                                                                  \
            mjs, &_js_arg_vals[_i], _js_args[_i].out, _js_args[_i].extra_data);                  \
    }

/**
 * @brief Prepends an error, sets the JS return value to `undefined` and returns
 * from the C function
 * @warning This macro executes `return;` by design
 */
#define JS_ERROR_AND_RETURN(mjs, error_code, ...)         \
    do {                                                  \
        mjs_prepend_errorf(mjs, error_code, __VA_ARGS__); \
        mjs_return(mjs, MJS_UNDEFINED);                   \
        return;                                           \
    } while(0)

typedef struct JsModules JsModules;

typedef void* (*JsModuleConstructor)(struct mjs* mjs, mjs_val_t* object, JsModules* modules);
typedef void (*JsModuleDestructor)(void* inst);

typedef struct {
    char* name;
    JsModuleConstructor create;
    JsModuleDestructor destroy;
    const ElfApiInterface* api_interface;
} JsModuleDescriptor;

JsModules* js_modules_create(struct mjs* mjs, CompositeApiResolver* resolver);

void js_modules_destroy(JsModules* modules);

mjs_val_t js_module_require(JsModules* modules, const char* name, size_t name_len);

/**
 * @brief Gets a module instance by its name
 * This is useful when a module wants to access a stateful API of another
 * module.
 * @returns Pointer to module context, NULL if the module is not instantiated
 */
void* js_module_get(JsModules* modules, const char* name);
