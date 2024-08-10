#include "../js_modules.h"
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/view_holder.h>
#include <toolbox/api_lock.h>

#define membersof(x) (sizeof(x) / sizeof(x[0]))

typedef struct {
    TextInput* text_input;
    ByteInput* byte_input;
    ViewHolder* view_holder;
    FuriApiLock lock;
    char* header;
    bool accepted;
} JsKeyboardInst;

static void ret_bad_args(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "%s", error);
    mjs_return(mjs, MJS_UNDEFINED);
}

static JsKeyboardInst* get_this_ctx(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsKeyboardInst* keyboard = mjs_get_ptr(mjs, obj_inst);
    furi_assert(keyboard);
    return keyboard;
}

static void keyboard_callback(void* context) {
    JsKeyboardInst* keyboard = (JsKeyboardInst*)context;
    keyboard->accepted = true;
    api_lock_unlock(keyboard->lock);
}

static void keyboard_exit(void* context) {
    JsKeyboardInst* keyboard = (JsKeyboardInst*)context;
    keyboard->accepted = false;
    api_lock_unlock(keyboard->lock);
}

static void js_keyboard_set_header(struct mjs* mjs) {
    JsKeyboardInst* keyboard = get_this_ctx(mjs);

    mjs_val_t header_arg = mjs_arg(mjs, 0);
    const char* header = mjs_get_string(mjs, &header_arg, NULL);
    if(!header) {
        ret_bad_args(mjs, "Header must be a string");
        return;
    }

    if(keyboard->header) {
        free(keyboard->header);
    }
    keyboard->header = strdup(header);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_keyboard_text(struct mjs* mjs) {
    JsKeyboardInst* keyboard = get_this_ctx(mjs);

    mjs_val_t input_length_arg = mjs_arg(mjs, 0);
    if(!mjs_is_number(input_length_arg)) {
        ret_bad_args(mjs, "Input length must be a number");
        return;
    }
    int32_t input_length = mjs_get_int32(mjs, input_length_arg);
    char* buffer = malloc(input_length);

    mjs_val_t default_text_arg = mjs_arg(mjs, 1);
    const char* default_text = mjs_get_string(mjs, &default_text_arg, NULL);
    bool clear_default = false;
    if(default_text) {
        strlcpy(buffer, default_text, input_length);
        mjs_val_t bool_obj = mjs_arg(mjs, 2);
        clear_default = mjs_get_bool(mjs, bool_obj);
    }

    if(keyboard->header) {
        text_input_set_header_text(keyboard->text_input, keyboard->header);
    }
    text_input_set_result_callback(
        keyboard->text_input, keyboard_callback, keyboard, buffer, input_length, clear_default);

    text_input_set_minimum_length(keyboard->text_input, 0);

    keyboard->lock = api_lock_alloc_locked();
    Gui* gui = furi_record_open(RECORD_GUI);
    keyboard->view_holder = view_holder_alloc();
    view_holder_attach_to_gui(keyboard->view_holder, gui);
    view_holder_set_back_callback(keyboard->view_holder, keyboard_exit, keyboard);

    view_holder_set_view(keyboard->view_holder, text_input_get_view(keyboard->text_input));
    api_lock_wait_unlock(keyboard->lock);

    view_holder_set_view(keyboard->view_holder, NULL);
    view_holder_free(keyboard->view_holder);

    furi_record_close(RECORD_GUI);
    api_lock_free(keyboard->lock);

    text_input_reset(keyboard->text_input);
    if(keyboard->header) {
        free(keyboard->header);
        keyboard->header = NULL;
    }
    if(keyboard->accepted) {
        mjs_return(mjs, mjs_mk_string(mjs, buffer, ~0, true));
    } else {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    free(buffer);
}

static void js_keyboard_byte(struct mjs* mjs) {
    JsKeyboardInst* keyboard = get_this_ctx(mjs);

    mjs_val_t input_length_arg = mjs_arg(mjs, 0);
    if(!mjs_is_number(input_length_arg)) {
        ret_bad_args(mjs, "Input length must be a number");
        return;
    }
    int32_t input_length = mjs_get_int32(mjs, input_length_arg);
    uint8_t* buffer = malloc(input_length);

    mjs_val_t default_data_arg = mjs_arg(mjs, 1);
    if(mjs_is_typed_array(default_data_arg)) {
        if(mjs_is_data_view(default_data_arg)) {
            default_data_arg = mjs_dataview_get_buf(mjs, default_data_arg);
        }
        size_t default_data_len = 0;
        char* default_data = mjs_array_buf_get_ptr(mjs, default_data_arg, &default_data_len);
        memcpy(buffer, (uint8_t*)default_data, MIN((size_t)input_length, default_data_len));
    }

    if(keyboard->header) {
        byte_input_set_header_text(keyboard->byte_input, keyboard->header);
    }
    byte_input_set_result_callback(
        keyboard->byte_input, keyboard_callback, NULL, keyboard, buffer, input_length);

    keyboard->lock = api_lock_alloc_locked();
    Gui* gui = furi_record_open(RECORD_GUI);
    keyboard->view_holder = view_holder_alloc();
    view_holder_attach_to_gui(keyboard->view_holder, gui);
    view_holder_set_back_callback(keyboard->view_holder, keyboard_exit, keyboard);

    view_holder_set_view(keyboard->view_holder, byte_input_get_view(keyboard->byte_input));
    api_lock_wait_unlock(keyboard->lock);

    view_holder_set_view(keyboard->view_holder, NULL);
    view_holder_free(keyboard->view_holder);

    furi_record_close(RECORD_GUI);
    api_lock_free(keyboard->lock);

    if(keyboard->header) {
        free(keyboard->header);
        keyboard->header = NULL;
    }
    byte_input_set_result_callback(keyboard->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(keyboard->byte_input, "");
    if(keyboard->accepted) {
        mjs_return(mjs, mjs_mk_array_buf(mjs, (char*)buffer, input_length));
    } else {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    free(buffer);
}

static void* js_keyboard_create(struct mjs* mjs, mjs_val_t* object) {
    JsKeyboardInst* keyboard = malloc(sizeof(JsKeyboardInst));
    mjs_val_t keyboard_obj = mjs_mk_object(mjs);
    mjs_set(mjs, keyboard_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, keyboard));
    mjs_set(mjs, keyboard_obj, "setHeader", ~0, MJS_MK_FN(js_keyboard_set_header));
    mjs_set(mjs, keyboard_obj, "text", ~0, MJS_MK_FN(js_keyboard_text));
    mjs_set(mjs, keyboard_obj, "byte", ~0, MJS_MK_FN(js_keyboard_byte));
    keyboard->byte_input = byte_input_alloc();
    keyboard->text_input = text_input_alloc();
    *object = keyboard_obj;
    return keyboard;
}

static void js_keyboard_destroy(void* inst) {
    JsKeyboardInst* keyboard = inst;
    byte_input_free(keyboard->byte_input);
    text_input_free(keyboard->text_input);
    free(keyboard);
}

static const JsModuleDescriptor js_keyboard_desc = {
    "keyboard",
    js_keyboard_create,
    js_keyboard_destroy,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_keyboard_desc,
};

const FlipperAppPluginDescriptor* js_keyboard_ep(void) {
    return &plugin_descriptor;
}
