#include <core/common_defines.h>
#include "../js_modules.h"
#include <notification/notification_messages.h>

static void js_notify(struct mjs* mjs, const NotificationSequence* sequence) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    NotificationApp* notification = mjs_get_ptr(mjs, obj_inst);
    furi_assert(notification);
    notification_message(notification, sequence);
}

static void js_notify_success(struct mjs* mjs) {
    js_notify(mjs, &sequence_success);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_notify_error(struct mjs* mjs) {
    js_notify(mjs, &sequence_error);
    mjs_return(mjs, MJS_UNDEFINED);
}

static const struct {
    const char* color_name;
    const NotificationSequence* sequence_short;
    const NotificationSequence* sequence_long;
} led_sequences[] = {
    {"blue", &sequence_blink_blue_10, &sequence_blink_blue_100},
    {"red", &sequence_blink_red_10, &sequence_blink_red_100},
    {"green", &sequence_blink_green_10, &sequence_blink_green_100},
    {"yellow", &sequence_blink_yellow_10, &sequence_blink_yellow_100},
    {"cyan", &sequence_blink_cyan_10, &sequence_blink_cyan_100},
    {"magenta", &sequence_blink_magenta_10, &sequence_blink_magenta_100},
};

static void js_notify_blink(struct mjs* mjs) {
    const NotificationSequence* sequence = NULL;
    do {
        size_t num_args = mjs_nargs(mjs);
        if(num_args != 2) {
            break;
        }
        mjs_val_t color_obj = mjs_arg(mjs, 0);
        mjs_val_t type_obj = mjs_arg(mjs, 1);
        if((!mjs_is_string(color_obj)) || (!mjs_is_string(type_obj))) break;

        size_t arg_len = 0;
        const char* arg_str = mjs_get_string(mjs, &color_obj, &arg_len);
        if((arg_len == 0) || (arg_str == NULL)) break;

        int32_t color_id = -1;
        for(size_t i = 0; i < COUNT_OF(led_sequences); i++) {
            size_t name_len = strlen(led_sequences[i].color_name);
            if(arg_len != name_len) continue;
            if(strncmp(arg_str, led_sequences[i].color_name, arg_len) == 0) {
                color_id = i;
                break;
            }
        }
        if(color_id == -1) break;

        arg_str = mjs_get_string(mjs, &type_obj, &arg_len);
        if((arg_len == 0) || (arg_str == NULL)) break;
        if(strncmp(arg_str, "short", arg_len) == 0) {
            sequence = led_sequences[color_id].sequence_short;
        } else if(strncmp(arg_str, "long", arg_len) == 0) {
            sequence = led_sequences[color_id].sequence_long;
        }
    } while(0);

    if(sequence == NULL) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
    } else {
        js_notify(mjs, sequence);
    }
    mjs_return(mjs, MJS_UNDEFINED);
}

static void* js_notification_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    UNUSED(modules);
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    mjs_val_t notify_obj = mjs_mk_object(mjs);
    mjs_set(mjs, notify_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, notification));
    mjs_set(mjs, notify_obj, "success", ~0, MJS_MK_FN(js_notify_success));
    mjs_set(mjs, notify_obj, "error", ~0, MJS_MK_FN(js_notify_error));
    mjs_set(mjs, notify_obj, "blink", ~0, MJS_MK_FN(js_notify_blink));
    *object = notify_obj;

    return notification;
}

static void js_notification_destroy(void* inst) {
    UNUSED(inst);
    furi_record_close(RECORD_NOTIFICATION);
}

static const JsModuleDescriptor js_notification_desc = {
    "notification",
    js_notification_create,
    js_notification_destroy,
    NULL,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_notification_desc,
};

const FlipperAppPluginDescriptor* js_notification_ep(void) {
    return &plugin_descriptor;
}
