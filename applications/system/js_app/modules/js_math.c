#include "../js_modules.h"
#include "furi_hal_random.h"

#define JS_MATH_PI (double)3.14159265358979323846
#define JS_MATH_E (double)2.7182818284590452354

static void ret_bad_args(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "%s", error);
    mjs_return(mjs, mjs_mk_undefined());
}

static bool check_arg_count(struct mjs* mjs, size_t count) {
    size_t num_args = mjs_nargs(mjs);
    if(num_args != count) {
        ret_bad_args(mjs, "Wrong argument count");
        return false;
    }
    return true;
}

void js_math_abs(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, x < 0 ? mjs_mk_number(mjs, -x) : mjs_arg(mjs, 0));
}

void js_math_acos(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    if(x < -1 || x > 1) {
        ret_bad_args(mjs, "Invalid input value for Math.acos");
        mjs_return(mjs, MJS_UNDEFINED);
    }
    mjs_return(mjs, mjs_mk_number(mjs, JS_MATH_PI / (double)2 - atan(x / sqrt(1 - x * x))));
}

void js_math_acosh(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    if(x < 1) {
        ret_bad_args(mjs, "Invalid input value for Math.acosh");
        mjs_return(mjs, MJS_UNDEFINED);
    }
    mjs_return(mjs, mjs_mk_number(mjs, log(x + sqrt(x * x - 1))));
}

void js_math_asin(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, atan(x / sqrt(1 - x * x))));
}

void js_math_asinh(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, log(x + sqrt(x * x + 1))));
}

void js_math_atan(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, atan(x)));
}

void js_math_atan2(struct mjs* mjs) {
    if(!check_arg_count(mjs, 2) || !mjs_is_number(mjs_arg(mjs, 0)) ||
       !mjs_is_number(mjs_arg(mjs, 1))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double y = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double x = mjs_get_double(mjs, mjs_arg(mjs, 1));
    mjs_return(mjs, mjs_mk_number(mjs, atan2(y, x)));
}

void js_math_atanh(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    if(x <= -1 || x >= 1) {
        ret_bad_args(mjs, "Invalid input value for Math.atanh");
        mjs_return(mjs, MJS_UNDEFINED);
    }
    mjs_return(mjs, mjs_mk_number(mjs, (double)0.5 * log((1 + x) / (1 - x))));
}

void js_math_cbrt(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, pow(x, 1.0 / 3.0)));
}

void js_math_ceil(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, (int)(x + (double)0.5)));
}

void js_math_clz32(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    unsigned int x = (unsigned int)mjs_get_int(mjs, mjs_arg(mjs, 0));
    int count = 0;
    while(x) {
        x >>= 1;
        count++;
    }
    mjs_return(mjs, mjs_mk_number(mjs, 32 - count));
}

void js_math_cos(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, cos(x)));
}

void js_math_exp(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double result = 1;
    double term = 1;
    for(int i = 1; i < 100; i++) {
        term *= x / i;
        result += term;
    }
    mjs_return(mjs, mjs_mk_number(mjs, result));
}

void js_math_floor(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, (int)x));
}

void js_math_log(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    if(x <= 0) {
        ret_bad_args(mjs, "Invalid input value for Math.log");
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double result = 0;
    while(x >= JS_MATH_E) {
        x /= JS_MATH_E;
        result++;
    }
    mjs_return(mjs, mjs_mk_number(mjs, result + log(x)));
}

void js_math_max(struct mjs* mjs) {
    if(!check_arg_count(mjs, 2) || !mjs_is_number(mjs_arg(mjs, 0)) ||
       !mjs_is_number(mjs_arg(mjs, 1))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double y = mjs_get_double(mjs, mjs_arg(mjs, 1));
    mjs_return(mjs, mjs_mk_number(mjs, x > y ? x : y));
}

void js_math_min(struct mjs* mjs) {
    if(!check_arg_count(mjs, 2) || !mjs_is_number(mjs_arg(mjs, 0)) ||
       !mjs_is_number(mjs_arg(mjs, 1))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double y = mjs_get_double(mjs, mjs_arg(mjs, 1));
    mjs_return(mjs, mjs_mk_number(mjs, x < y ? x : y));
}

void js_math_pow(struct mjs* mjs) {
    if(!check_arg_count(mjs, 2) || !mjs_is_number(mjs_arg(mjs, 0)) ||
       !mjs_is_number(mjs_arg(mjs, 1))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double base = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double exponent = mjs_get_double(mjs, mjs_arg(mjs, 1));
    double result = 1;
    for(int i = 0; i < exponent; i++) {
        result *= base;
    }
    mjs_return(mjs, mjs_mk_number(mjs, result));
}

void js_math_random(struct mjs* mjs) {
    if(!check_arg_count(mjs, 0)) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    const uint32_t random_val = furi_hal_random_get();
    double rnd = (double)random_val / RAND_MAX;
    mjs_return(mjs, mjs_mk_number(mjs, rnd));
}

void js_math_sign(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, x == 0 ? 0 : (x < 0 ? -1 : 1)));
}

void js_math_sin(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double result = x;
    double term = x;
    for(int i = 1; i < 10; i++) {
        term *= -x * x / ((2 * i) * (2 * i + 1));
        result += term;
    }
    mjs_return(mjs, mjs_mk_number(mjs, result));
}

void js_math_sqrt(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    if(x < 0) {
        ret_bad_args(mjs, "Invalid input value for Math.sqrt");
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double result = 1;
    while(result * result < x) {
        result += (double)0.001;
    }
    mjs_return(mjs, mjs_mk_number(mjs, result));
}

void js_math_trunc(struct mjs* mjs) {
    if(!check_arg_count(mjs, 1) || !mjs_is_number(mjs_arg(mjs, 0))) {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, x < 0 ? ceil(x) : floor(x)));
}

static void* js_math_create(struct mjs* mjs, mjs_val_t* object) {
    mjs_val_t math_obj = mjs_mk_object(mjs);
    mjs_set(mjs, math_obj, "abs", ~0, MJS_MK_FN(js_math_abs));
    mjs_set(mjs, math_obj, "acos", ~0, MJS_MK_FN(js_math_acos));
    mjs_set(mjs, math_obj, "acosh", ~0, MJS_MK_FN(js_math_acosh));
    mjs_set(mjs, math_obj, "asin", ~0, MJS_MK_FN(js_math_asin));
    mjs_set(mjs, math_obj, "asinh", ~0, MJS_MK_FN(js_math_asinh));
    mjs_set(mjs, math_obj, "atan", ~0, MJS_MK_FN(js_math_atan));
    mjs_set(mjs, math_obj, "atan2", ~0, MJS_MK_FN(js_math_atan2));
    mjs_set(mjs, math_obj, "atanh", ~0, MJS_MK_FN(js_math_atanh));
    mjs_set(mjs, math_obj, "cbrt", ~0, MJS_MK_FN(js_math_cbrt));
    mjs_set(mjs, math_obj, "ceil", ~0, MJS_MK_FN(js_math_ceil));
    mjs_set(mjs, math_obj, "clz32", ~0, MJS_MK_FN(js_math_clz32));
    mjs_set(mjs, math_obj, "cos", ~0, MJS_MK_FN(js_math_cos));
    mjs_set(mjs, math_obj, "exp", ~0, MJS_MK_FN(js_math_exp));
    mjs_set(mjs, math_obj, "floor", ~0, MJS_MK_FN(js_math_floor));
    mjs_set(mjs, math_obj, "log", ~0, MJS_MK_FN(js_math_log));
    mjs_set(mjs, math_obj, "max", ~0, MJS_MK_FN(js_math_max));
    mjs_set(mjs, math_obj, "min", ~0, MJS_MK_FN(js_math_min));
    mjs_set(mjs, math_obj, "pow", ~0, MJS_MK_FN(js_math_pow));
    mjs_set(mjs, math_obj, "random", ~0, MJS_MK_FN(js_math_random));
    mjs_set(mjs, math_obj, "sign", ~0, MJS_MK_FN(js_math_sign));
    mjs_set(mjs, math_obj, "sin", ~0, MJS_MK_FN(js_math_sin));
    mjs_set(mjs, math_obj, "sqrt", ~0, MJS_MK_FN(js_math_sqrt));
    mjs_set(mjs, math_obj, "trunc", ~0, MJS_MK_FN(js_math_trunc));
    mjs_set(mjs, math_obj, "PI", ~0, mjs_mk_number(mjs, JS_MATH_PI));
    mjs_set(mjs, math_obj, "E", ~0, mjs_mk_number(mjs, JS_MATH_E));
    *object = math_obj;
    return (void*)1;
}

static const JsModuleDescriptor js_math_desc = {
    "math",
    js_math_create,
    NULL,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_math_desc,
};

const FlipperAppPluginDescriptor* js_math_ep(void) {
    return &plugin_descriptor;
}