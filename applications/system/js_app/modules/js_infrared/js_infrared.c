#include "../../js_modules.h"
#include <infrared_worker.h>
#include <lib/infrared/signal/infrared_signal.h>
#include <lib/infrared/worker/infrared_transmit.h>

#define TAG "JsMath"

static void ret_bad_args(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "%s", error);
    mjs_return(mjs, MJS_UNDEFINED);
}

void js_send_protocol_signal(struct mjs* mjs) {
    size_t num_args = mjs_nargs(mjs);
    if(num_args < 3 || num_args > 4) {
        ret_bad_args(mjs, "Wrong argument count");
        return;
    }
    if(!mjs_is_string(mjs_arg(mjs, 0)) || !mjs_is_number(mjs_arg(mjs, 1)) ||
       !mjs_is_number(mjs_arg(mjs, 2)) || (num_args == 4 && !mjs_is_object(mjs_arg(mjs, 3)))) {
        ret_bad_args(mjs, "Wrong argument type");
        return;
    }
    bool repeat = false;
    int times = 1;
    if(num_args == 4) {
        mjs_val_t options_obj = mjs_arg(mjs, 3);

        mjs_val_t repeat_val = mjs_get(mjs, options_obj, "repeat", ~0);
        if(mjs_is_boolean(repeat_val)) {
            repeat = mjs_get_bool(mjs, repeat_val);
        } else if(!mjs_is_undefined(repeat_val)) {
            ret_bad_args(mjs, "Wrong 'repeat' option type");
            return;
        }

        mjs_val_t times_val = mjs_get(mjs, options_obj, "times", ~0);
        if(mjs_is_number(times_val)) {
            times = mjs_get_int(mjs, times_val);
        } else if(!mjs_is_undefined(times_val)) {
            ret_bad_args(mjs, "Wrong 'times' option type");
            return;
        }
    }

    InfraredMessage message;
    message.repeat = repeat;
    mjs_val_t protocol_arg = mjs_arg(mjs, 0);
    message.protocol = infrared_get_protocol_by_name(mjs_get_cstring(mjs, &protocol_arg));
    message.address = mjs_get_int(mjs, mjs_arg(mjs, 1));
    message.command = mjs_get_int(mjs, mjs_arg(mjs, 2));
    infrared_send(&message, times);
}

void js_send_raw_signal(struct mjs* mjs) {
    size_t num_args = mjs_nargs(mjs);
    if(num_args < 1 || num_args > 3) {
        ret_bad_args(mjs, "Wrong argument count");
        return;
    }
    if(!mjs_is_array(mjs_arg(mjs, 0)) || (num_args > 1 && !mjs_is_boolean(mjs_arg(mjs, 1))) ||
       (num_args > 2 && !mjs_is_object(mjs_arg(mjs, 2)))) {
        ret_bad_args(mjs, "Wrong argument type");
        return;
    }
    int array_length = mjs_array_length(mjs, mjs_arg(mjs, 0));
    uint32_t timings[array_length];
    for(int i = 0; i < array_length; i++) {
        mjs_val_t elem = mjs_array_get(mjs, mjs_arg(mjs, 0), i);
        if(!mjs_is_number(elem)) {
            ret_bad_args(mjs, "Timings array must contain only numbers");
            return;
        }
        timings[i] = mjs_get_int(mjs, elem);
    }

    bool start_from_mark = true;
    if(num_args > 1) {
        start_from_mark = mjs_get_bool(mjs, mjs_arg(mjs, 1));
    }

    if(num_args > 2) {
        mjs_val_t options_obj = mjs_arg(mjs, 2);

        mjs_val_t frequency_val = mjs_get(mjs, options_obj, "frequency", ~0);
        if(!mjs_is_number(frequency_val)) {
            ret_bad_args(mjs, "Wrong 'frequency' option type");
            return;
        }

        mjs_val_t duty_val = mjs_get(mjs, options_obj, "dutyCycle", ~0);
        if(!mjs_is_number(duty_val)) {
            ret_bad_args(mjs, "Wrong 'dutyCycle' option type");
            return;
        }
        uint32_t frequency = mjs_get_int(mjs, frequency_val);
        float duty_cycle = mjs_get_double(mjs, duty_val);
        infrared_send_raw_ext(timings, array_length, start_from_mark, frequency, duty_cycle);
    } else {
        infrared_send_raw(timings, array_length, start_from_mark);
    }
    return;
}

static void* js_infrared_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    UNUSED(modules);
    mjs_val_t infrared_object = mjs_mk_object(mjs);
    mjs_set(mjs, infrared_object, "sendSignal", ~0, MJS_MK_FN(js_send_protocol_signal));
    mjs_set(mjs, infrared_object, "sendRawSignal", ~0, MJS_MK_FN(js_send_raw_signal));
    *object = infrared_object;
    return (void*)1;
}

static const JsModuleDescriptor js_infrared_desc = {
    "infrared",
    js_infrared_create,
    NULL,
    NULL,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_infrared_desc,
};

const FlipperAppPluginDescriptor* js_infrared_ep(void) {
    return &plugin_descriptor;
}
