#include <core/common_defines.h>
#include "../js_modules.h"
#include <dialogs/dialogs.h>

static bool js_dialog_msg_parse_params(struct mjs* mjs, const char** hdr, const char** msg) {
    size_t num_args = mjs_nargs(mjs);
    if(num_args != 2) {
        return false;
    }
    mjs_val_t header_obj = mjs_arg(mjs, 0);
    mjs_val_t msg_obj = mjs_arg(mjs, 1);
    if((!mjs_is_string(header_obj)) || (!mjs_is_string(msg_obj))) {
        return false;
    }

    size_t arg_len = 0;
    *hdr = mjs_get_string(mjs, &header_obj, &arg_len);
    if(arg_len == 0) {
        *hdr = NULL;
    }

    *msg = mjs_get_string(mjs, &msg_obj, &arg_len);
    if(arg_len == 0) {
        *msg = NULL;
    }

    return true;
}

static void js_dialog_message(struct mjs* mjs) {
    const char* dialog_header = NULL;
    const char* dialog_msg = NULL;
    if(!js_dialog_msg_parse_params(mjs, &dialog_header, &dialog_msg)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_buttons(message, NULL, "OK", NULL);
    if(dialog_header) {
        dialog_message_set_header(message, dialog_header, 64, 3, AlignCenter, AlignTop);
    }
    if(dialog_msg) {
        dialog_message_set_text(message, dialog_msg, 64, 26, AlignCenter, AlignTop);
    }
    DialogMessageButton result = dialog_message_show(dialogs, message);
    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);
    mjs_return(mjs, mjs_mk_boolean(mjs, result == DialogMessageButtonCenter));
}

static void js_dialog_custom(struct mjs* mjs) {
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* message = dialog_message_alloc();

    bool params_correct = false;

    do {
        if(mjs_nargs(mjs) != 1) {
            break;
        }
        mjs_val_t params_obj = mjs_arg(mjs, 0);
        if(!mjs_is_object(params_obj)) {
            break;
        }

        mjs_val_t text_obj = mjs_get(mjs, params_obj, "header", ~0);
        size_t arg_len = 0;
        const char* text_str = mjs_get_string(mjs, &text_obj, &arg_len);
        if(arg_len == 0) {
            text_str = NULL;
        }
        if(text_str) {
            dialog_message_set_header(message, text_str, 64, 3, AlignCenter, AlignTop);
        }

        text_obj = mjs_get(mjs, params_obj, "text", ~0);
        text_str = mjs_get_string(mjs, &text_obj, &arg_len);
        if(arg_len == 0) {
            text_str = NULL;
        }
        if(text_str) {
            dialog_message_set_text(message, text_str, 64, 26, AlignCenter, AlignTop);
        }

        mjs_val_t btn_obj[3] = {
            mjs_get(mjs, params_obj, "button_left", ~0),
            mjs_get(mjs, params_obj, "button_center", ~0),
            mjs_get(mjs, params_obj, "button_right", ~0),
        };
        const char* btn_text[3] = {NULL, NULL, NULL};

        for(uint8_t i = 0; i < 3; i++) {
            if(!mjs_is_string(btn_obj[i])) {
                continue;
            }
            btn_text[i] = mjs_get_string(mjs, &btn_obj[i], &arg_len);
            if(arg_len == 0) {
                btn_text[i] = NULL;
            }
        }

        dialog_message_set_buttons(message, btn_text[0], btn_text[1], btn_text[2]);

        DialogMessageButton result = dialog_message_show(dialogs, message);
        mjs_val_t return_obj = MJS_UNDEFINED;
        if(result == DialogMessageButtonLeft) {
            return_obj = mjs_mk_string(mjs, btn_text[0], ~0, true);
        } else if(result == DialogMessageButtonCenter) {
            return_obj = mjs_mk_string(mjs, btn_text[1], ~0, true);
        } else if(result == DialogMessageButtonRight) {
            return_obj = mjs_mk_string(mjs, btn_text[2], ~0, true);
        } else {
            return_obj = mjs_mk_string(mjs, "", ~0, true);
        }

        mjs_return(mjs, return_obj);
        params_correct = true;
    } while(0);

    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);

    if(!params_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
    }
}

static void* js_dialog_create(struct mjs* mjs, mjs_val_t* object) {
    mjs_val_t dialog_obj = mjs_mk_object(mjs);
    mjs_set(mjs, dialog_obj, "message", ~0, MJS_MK_FN(js_dialog_message));
    mjs_set(mjs, dialog_obj, "custom", ~0, MJS_MK_FN(js_dialog_custom));
    *object = dialog_obj;

    return (void*)1;
}

static const JsModuleDescriptor js_dialog_desc = {
    "dialog",
    js_dialog_create,
    NULL,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_dialog_desc,
};

const FlipperAppPluginDescriptor* js_dialog_ep(void) {
    return &plugin_descriptor;
}
