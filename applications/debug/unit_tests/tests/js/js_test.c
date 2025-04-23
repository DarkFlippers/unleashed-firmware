#include "../test.h" // IWYU pragma: keep

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_random.h>

#include <storage/storage.h>
#include <applications/system/js_app/js_thread.h>
#include <applications/system/js_app/js_value.h>

#include <stdint.h>

#define TAG "JsUnitTests"

#define JS_SCRIPT_PATH(name) EXT_PATH("unit_tests/js/" name ".js")

typedef enum {
    JsTestsFinished = 1,
    JsTestsError = 2,
} JsTestFlag;

typedef struct {
    FuriEventFlag* event_flags;
    FuriString* error_string;
} JsTestCallbackContext;

static void js_test_callback(JsThreadEvent event, const char* msg, void* param) {
    JsTestCallbackContext* context = param;
    if(event == JsThreadEventPrint) {
        FURI_LOG_I("js_test", "%s", msg);
    } else if(event == JsThreadEventError || event == JsThreadEventErrorTrace) {
        context->error_string = furi_string_alloc_set_str(msg);
        furi_event_flag_set(context->event_flags, JsTestsFinished | JsTestsError);
    } else if(event == JsThreadEventDone) {
        furi_event_flag_set(context->event_flags, JsTestsFinished);
    }
}

static void js_test_run(const char* script_path) {
    JsTestCallbackContext* context = malloc(sizeof(JsTestCallbackContext));
    context->event_flags = furi_event_flag_alloc();

    JsThread* thread = js_thread_run(script_path, js_test_callback, context);
    uint32_t flags = furi_event_flag_wait(
        context->event_flags, JsTestsFinished, FuriFlagWaitAny, FuriWaitForever);
    if(flags & FuriFlagError) {
        // getting the flags themselves should not fail
        furi_crash();
    }

    FuriString* error_string = context->error_string;

    js_thread_stop(thread);
    furi_event_flag_free(context->event_flags);
    free(context);

    if(flags & JsTestsError) {
        // memory leak: not freeing the FuriString if the tests fail,
        // because mu_fail executes a return
        //
        // who cares tho?
        mu_fail(furi_string_get_cstr(error_string));
    }
}

MU_TEST(js_test_basic) {
    js_test_run(JS_SCRIPT_PATH("basic"));
}
MU_TEST(js_test_math) {
    js_test_run(JS_SCRIPT_PATH("math"));
}
MU_TEST(js_test_event_loop) {
    js_test_run(JS_SCRIPT_PATH("event_loop"));
}
MU_TEST(js_test_storage) {
    js_test_run(JS_SCRIPT_PATH("storage"));
}

static void js_value_test_compatibility_matrix(struct mjs* mjs) {
    static const JsValueType types[] = {
        JsValueTypeAny,
        JsValueTypeAnyArray,
        JsValueTypeAnyObject,
        JsValueTypeFunction,
        JsValueTypeRawPointer,
        JsValueTypeInt32,
        JsValueTypeDouble,
        JsValueTypeString,
        JsValueTypeBool,
    };

    mjs_val_t values[] = {
        mjs_mk_undefined(),
        mjs_mk_foreign(mjs, (void*)0xDEADBEEF),
        mjs_mk_array(mjs),
        mjs_mk_object(mjs),
        mjs_mk_number(mjs, 123.456),
        mjs_mk_string(mjs, "test", ~0, false),
        mjs_mk_boolean(mjs, true),
    };

// for proper matrix formatting and better readability
#define YES true
#define NO_ false
    static const bool success_matrix[COUNT_OF(types)][COUNT_OF(values)] = {
        //                                      types:
        {YES, YES, YES, YES, YES, YES, YES}, // any
        {NO_, NO_, YES, NO_, NO_, NO_, NO_}, // array
        {NO_, NO_, YES, YES, NO_, NO_, NO_}, // obj
        {NO_, NO_, NO_, NO_, NO_, NO_, NO_}, // fn
        {NO_, YES, NO_, NO_, NO_, NO_, NO_}, // ptr
        {NO_, NO_, NO_, NO_, YES, NO_, NO_}, // int32
        {NO_, NO_, NO_, NO_, YES, NO_, NO_}, // double
        {NO_, NO_, NO_, NO_, NO_, YES, NO_}, // str
        {NO_, NO_, NO_, NO_, NO_, NO_, YES}, // bool
        //
        //und ptr  arr  obj  num  str  bool <- values
    };
#undef NO_
#undef YES

    for(size_t i = 0; i < COUNT_OF(types); i++) {
        for(size_t j = 0; j < COUNT_OF(values); j++) {
            const JsValueDeclaration declaration = {
                .type = types[i],
                .n_children = 0,
            };
            // we only care about the status, not the result. double has the largest size out of
            // all the results
            uint8_t result[sizeof(double)];
            JsValueParseStatus status;
            JS_VALUE_PARSE(
                mjs,
                JS_VALUE_PARSE_SOURCE_VALUE(&declaration),
                JsValueParseFlagNone,
                &status,
                &values[j],
                result);
            if((status == JsValueParseStatusOk) != success_matrix[i][j]) {
                FURI_LOG_E(TAG, "type %zu, value %zu", i, j);
                mu_fail("see serial logs");
            }
        }
    }
}

static void js_value_test_literal(struct mjs* mjs) {
    static const JsValueType types[] = {
        JsValueTypeAny,
        JsValueTypeAnyArray,
        JsValueTypeAnyObject,
    };

    mjs_val_t values[] = {
        mjs_mk_undefined(),
        mjs_mk_array(mjs),
        mjs_mk_object(mjs),
    };

    mu_assert_int_eq(COUNT_OF(types), COUNT_OF(values));
    for(size_t i = 0; i < COUNT_OF(types); i++) {
        const JsValueDeclaration declaration = {
            .type = types[i],
            .n_children = 0,
        };
        mjs_val_t result;
        JsValueParseStatus status;
        JS_VALUE_PARSE(
            mjs,
            JS_VALUE_PARSE_SOURCE_VALUE(&declaration),
            JsValueParseFlagNone,
            &status,
            &values[i],
            &result);
        mu_assert_int_eq(JsValueParseStatusOk, status);
        mu_assert(result == values[i], "wrong result");
    }
}

static void js_value_test_primitive(
    struct mjs* mjs,
    JsValueType type,
    const void* c_value,
    size_t c_value_size,
    mjs_val_t js_val) {
    const JsValueDeclaration declaration = {
        .type = type,
        .n_children = 0,
    };
    uint8_t result[c_value_size];
    JsValueParseStatus status;
    JS_VALUE_PARSE(
        mjs,
        JS_VALUE_PARSE_SOURCE_VALUE(&declaration),
        JsValueParseFlagNone,
        &status,
        &js_val,
        result);
    mu_assert_int_eq(JsValueParseStatusOk, status);
    if(type == JsValueTypeString) {
        const char* result_str = *(const char**)&result;
        mu_assert_string_eq(c_value, result_str);
    } else {
        mu_assert_mem_eq(c_value, result, c_value_size);
    }
}

static void js_value_test_primitives(struct mjs* mjs) {
    int32_t i32 = 123;
    js_value_test_primitive(mjs, JsValueTypeInt32, &i32, sizeof(i32), mjs_mk_number(mjs, i32));

    double dbl = 123.456;
    js_value_test_primitive(mjs, JsValueTypeDouble, &dbl, sizeof(dbl), mjs_mk_number(mjs, dbl));

    const char* str = "test";
    js_value_test_primitive(
        mjs, JsValueTypeString, str, strlen(str) + 1, mjs_mk_string(mjs, str, ~0, false));

    bool boolean = true;
    js_value_test_primitive(
        mjs, JsValueTypeBool, &boolean, sizeof(boolean), mjs_mk_boolean(mjs, boolean));
}

static uint32_t
    js_value_test_enum(struct mjs* mjs, const JsValueDeclaration* decl, const char* value) {
    mjs_val_t str = mjs_mk_string(mjs, value, ~0, false);
    uint32_t result;
    JsValueParseStatus status;
    JS_VALUE_PARSE(
        mjs, JS_VALUE_PARSE_SOURCE_VALUE(decl), JsValueParseFlagNone, &status, &str, &result);
    if(status != JsValueParseStatusOk) return 0;
    return result;
}

static void js_value_test_enums(struct mjs* mjs) {
    static const JsValueEnumVariant enum_1_variants[] = {
        {"variant 1", 1},
        {"variant 2", 2},
        {"variant 3", 3},
    };
    static const JsValueDeclaration enum_1 = JS_VALUE_ENUM(uint32_t, enum_1_variants);

    static const JsValueEnumVariant enum_2_variants[] = {
        {"read", 4},
        {"write", 8},
    };
    static const JsValueDeclaration enum_2 = JS_VALUE_ENUM(uint32_t, enum_2_variants);

    mu_assert_int_eq(1, js_value_test_enum(mjs, &enum_1, "variant 1"));
    mu_assert_int_eq(2, js_value_test_enum(mjs, &enum_1, "variant 2"));
    mu_assert_int_eq(3, js_value_test_enum(mjs, &enum_1, "variant 3"));
    mu_assert_int_eq(0, js_value_test_enum(mjs, &enum_1, "not a thing"));

    mu_assert_int_eq(0, js_value_test_enum(mjs, &enum_2, "variant 1"));
    mu_assert_int_eq(0, js_value_test_enum(mjs, &enum_2, "variant 2"));
    mu_assert_int_eq(0, js_value_test_enum(mjs, &enum_2, "variant 3"));
    mu_assert_int_eq(0, js_value_test_enum(mjs, &enum_2, "not a thing"));
    mu_assert_int_eq(4, js_value_test_enum(mjs, &enum_2, "read"));
    mu_assert_int_eq(8, js_value_test_enum(mjs, &enum_2, "write"));
}

static void js_value_test_object(struct mjs* mjs) {
    static const JsValueDeclaration int_decl = JS_VALUE_SIMPLE(JsValueTypeInt32);

    static const JsValueDeclaration str_decl = JS_VALUE_SIMPLE(JsValueTypeString);

    static const JsValueEnumVariant enum_variants[] = {
        {"variant 1", 1},
        {"variant 2", 2},
        {"variant 3", 3},
    };
    static const JsValueDeclaration enum_decl = JS_VALUE_ENUM(uint32_t, enum_variants);

    static const JsValueObjectField fields[] = {
        {"int", &int_decl},
        {"str", &str_decl},
        {"enum", &enum_decl},
    };
    static const JsValueDeclaration object_decl = JS_VALUE_OBJECT(fields);

    mjs_val_t object = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, object) {
        JS_FIELD("str", mjs_mk_string(mjs, "Helloooo!", ~0, false));
        JS_FIELD("int", mjs_mk_number(mjs, 123));
        JS_FIELD("enum", mjs_mk_string(mjs, "variant 2", ~0, false));
    }

    const char* result_str;
    int32_t result_int;
    uint32_t result_enum;
    JsValueParseStatus status;
    JS_VALUE_PARSE(
        mjs,
        JS_VALUE_PARSE_SOURCE_VALUE(&object_decl),
        JsValueParseFlagNone,
        &status,
        &object,
        &result_int,
        &result_str,
        &result_enum);
    mu_assert_int_eq(JsValueParseStatusOk, status);
    mu_assert_string_eq("Helloooo!", result_str);
    mu_assert_int_eq(123, result_int);
    mu_assert_int_eq(2, result_enum);
}

static void js_value_test_default(struct mjs* mjs) {
    static const JsValueDeclaration int_decl =
        JS_VALUE_SIMPLE_W_DEFAULT(JsValueTypeInt32, int32_val, 123);
    static const JsValueDeclaration str_decl = JS_VALUE_SIMPLE(JsValueTypeString);

    static const JsValueObjectField fields[] = {
        {"int", &int_decl},
        {"str", &str_decl},
    };
    static const JsValueDeclaration object_decl = JS_VALUE_OBJECT(fields);

    mjs_val_t object = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, object) {
        JS_FIELD("str", mjs_mk_string(mjs, "Helloooo!", ~0, false));
        JS_FIELD("int", mjs_mk_undefined());
    }

    const char* result_str;
    int32_t result_int;
    JsValueParseStatus status;
    JS_VALUE_PARSE(
        mjs,
        JS_VALUE_PARSE_SOURCE_VALUE(&object_decl),
        JsValueParseFlagNone,
        &status,
        &object,
        &result_int,
        &result_str);
    mu_assert_string_eq("Helloooo!", result_str);
    mu_assert_int_eq(123, result_int);
}

static void js_value_test_args_fn(struct mjs* mjs) {
    static const JsValueDeclaration arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeInt32),
        JS_VALUE_SIMPLE(JsValueTypeInt32),
        JS_VALUE_SIMPLE(JsValueTypeInt32),
    };
    static const JsValueArguments args = JS_VALUE_ARGS(arg_list);

    int32_t a, b, c;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &args, &a, &b, &c);

    mu_assert_int_eq(123, a);
    mu_assert_int_eq(456, b);
    mu_assert_int_eq(-420, c);
}

static void js_value_test_args(struct mjs* mjs) {
    mjs_val_t function = MJS_MK_FN(js_value_test_args_fn);

    mjs_val_t result;
    mjs_val_t args[] = {
        mjs_mk_number(mjs, 123),
        mjs_mk_number(mjs, 456),
        mjs_mk_number(mjs, -420),
    };
    mu_assert_int_eq(
        MJS_OK, mjs_apply(mjs, &result, function, MJS_UNDEFINED, COUNT_OF(args), args));
}

MU_TEST(js_value_test) {
    struct mjs* mjs = mjs_create(NULL);

    js_value_test_compatibility_matrix(mjs);
    js_value_test_literal(mjs);
    js_value_test_primitives(mjs);
    js_value_test_enums(mjs);
    js_value_test_object(mjs);
    js_value_test_default(mjs);
    js_value_test_args(mjs);

    mjs_destroy(mjs);
}

MU_TEST_SUITE(test_js) {
    MU_RUN_TEST(js_value_test);
    MU_RUN_TEST(js_test_basic);
    MU_RUN_TEST(js_test_math);
    MU_RUN_TEST(js_test_event_loop);
    MU_RUN_TEST(js_test_storage);
}

int run_minunit_test_js(void) {
    MU_RUN_SUITE(test_js);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_js)
