#include "../js_modules.h" // IWYU pragma: keep
#include <core/common_defines.h>
#include <furi_hal_version.h>
#include <power/power_service/power.h>

#define TAG "JsTests"

static void js_tests_fail(struct mjs* mjs) {
    furi_check(mjs_nargs(mjs) == 1);
    mjs_val_t message_arg = mjs_arg(mjs, 0);
    const char* message = mjs_get_string(mjs, &message_arg, NULL);
    furi_check(message);
    mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "%s", message);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_tests_assert_eq(struct mjs* mjs) {
    furi_check(mjs_nargs(mjs) == 2);

    mjs_val_t expected_arg = mjs_arg(mjs, 0);
    mjs_val_t result_arg = mjs_arg(mjs, 1);

    if(mjs_is_number(expected_arg) && mjs_is_number(result_arg)) {
        int32_t expected = mjs_get_int32(mjs, expected_arg);
        int32_t result = mjs_get_int32(mjs, result_arg);
        if(expected == result) {
            FURI_LOG_T(TAG, "eq passed (exp=%ld res=%ld)", expected, result);
        } else {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "expected %d, found %d", expected, result);
        }
    } else if(mjs_is_string(expected_arg) && mjs_is_string(result_arg)) {
        const char* expected = mjs_get_string(mjs, &expected_arg, NULL);
        const char* result = mjs_get_string(mjs, &result_arg, NULL);
        if(strcmp(expected, result) == 0) {
            FURI_LOG_T(TAG, "eq passed (exp=\"%s\" res=\"%s\")", expected, result);
        } else {
            mjs_prepend_errorf(
                mjs, MJS_INTERNAL_ERROR, "expected \"%s\", found \"%s\"", expected, result);
        }
    } else if(mjs_is_boolean(expected_arg) && mjs_is_boolean(result_arg)) {
        bool expected = mjs_get_bool(mjs, expected_arg);
        bool result = mjs_get_bool(mjs, result_arg);
        if(expected == result) {
            FURI_LOG_T(
                TAG,
                "eq passed (exp=%s res=%s)",
                expected ? "true" : "false",
                result ? "true" : "false");
        } else {
            mjs_prepend_errorf(
                mjs,
                MJS_INTERNAL_ERROR,
                "expected %s, found %s",
                expected ? "true" : "false",
                result ? "true" : "false");
        }
    } else {
        JS_ERROR_AND_RETURN(
            mjs,
            MJS_INTERNAL_ERROR,
            "type mismatch (expected %s, result %s)",
            mjs_typeof(expected_arg),
            mjs_typeof(result_arg));
    }

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_tests_assert_float_close(struct mjs* mjs) {
    furi_check(mjs_nargs(mjs) == 3);

    mjs_val_t expected_arg = mjs_arg(mjs, 0);
    mjs_val_t result_arg = mjs_arg(mjs, 1);
    mjs_val_t epsilon_arg = mjs_arg(mjs, 2);
    furi_check(mjs_is_number(expected_arg));
    furi_check(mjs_is_number(result_arg));
    furi_check(mjs_is_number(epsilon_arg));
    double expected = mjs_get_double(mjs, expected_arg);
    double result = mjs_get_double(mjs, result_arg);
    double epsilon = mjs_get_double(mjs, epsilon_arg);

    if(ABS(expected - result) > epsilon) {
        mjs_prepend_errorf(
            mjs,
            MJS_INTERNAL_ERROR,
            "expected %f found %f (tolerance=%f)",
            expected,
            result,
            epsilon);
    }

    mjs_return(mjs, MJS_UNDEFINED);
}

void* js_tests_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    UNUSED(modules);
    mjs_val_t tests_obj = mjs_mk_object(mjs);
    mjs_set(mjs, tests_obj, "fail", ~0, MJS_MK_FN(js_tests_fail));
    mjs_set(mjs, tests_obj, "assert_eq", ~0, MJS_MK_FN(js_tests_assert_eq));
    mjs_set(mjs, tests_obj, "assert_float_close", ~0, MJS_MK_FN(js_tests_assert_float_close));
    *object = tests_obj;

    return (void*)1;
}
