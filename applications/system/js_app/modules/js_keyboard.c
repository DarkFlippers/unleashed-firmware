#include "../js_modules.h"
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/view_dispatcher.h>

#define membersof(x) (sizeof(x) / sizeof(x[0]))

typedef struct {
    char* data;
    TextInput* text_input;
    ByteInput* byte_input;
    ViewDispatcher* view_dispatcher;
    uint8_t* byteinput;
} JsKeyboardInst;

typedef enum {
    JsKeyboardViewTextInput,
    JsKeyboardViewByteInput,
} JsKeyboardView;

static void ret_bad_args(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "%s", error);
    mjs_return(mjs, MJS_UNDEFINED);
}

static bool get_str_arg(struct mjs* mjs, size_t index, const char** value, bool error) {
    mjs_val_t str_obj = mjs_arg(mjs, index);
    if(!mjs_is_string(str_obj)) {
        if(error) ret_bad_args(mjs, "Argument must be a string");
        return false;
    }
    size_t str_len = 0;
    *value = mjs_get_string(mjs, &str_obj, &str_len);
    if((str_len == 0) || (*value == NULL)) {
        if(error) ret_bad_args(mjs, "Bad string argument");
        return false;
    }
    return true;
}

static bool get_int_arg(struct mjs* mjs, size_t index, size_t* value, bool error) {
    mjs_val_t int_obj = mjs_arg(mjs, index);
    if(!mjs_is_number(int_obj)) {
        if(error) ret_bad_args(mjs, "Argument must be a number");
        return false;
    }
    *value = mjs_get_int(mjs, int_obj);
    return true;
}

static JsKeyboardInst* get_this_ctx(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsKeyboardInst* storage = mjs_get_ptr(mjs, obj_inst);
    furi_assert(storage);
    return storage;
}

void text_input_callback(void* context) {
    JsKeyboardInst* keyboard = (JsKeyboardInst*)context;
    view_dispatcher_stop(keyboard->view_dispatcher);
}

void byte_input_callback(void* context) {
    JsKeyboardInst* keyboard = (JsKeyboardInst*)context;
    view_dispatcher_stop(keyboard->view_dispatcher);
}

static void js_keyboard_set_header(struct mjs* mjs) {
    JsKeyboardInst* keyboard = get_this_ctx(mjs);

    const char* header;
    if(!get_str_arg(mjs, 0, &header, true)) return;

    text_input_set_header_text(keyboard->text_input, header);
    byte_input_set_header_text(keyboard->byte_input, header);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_keyboard_text(struct mjs* mjs) {
    JsKeyboardInst* keyboard = get_this_ctx(mjs);

    size_t input_length;
    if(!get_int_arg(mjs, 0, &input_length, true)) return;
    char* buffer = malloc(input_length);

    const char* default_text = "";
    bool clear_default = false;
    if(get_str_arg(mjs, 1, &default_text, false)) {
        strlcpy(buffer, default_text, input_length);
        mjs_val_t bool_obj = mjs_arg(mjs, 2);
        clear_default = mjs_get_bool(mjs, bool_obj);
    }

    view_dispatcher_attach_to_gui(
        keyboard->view_dispatcher, furi_record_open(RECORD_GUI), ViewDispatcherTypeFullscreen);
    furi_record_close(RECORD_GUI);

    text_input_set_result_callback(
        keyboard->text_input, text_input_callback, keyboard, buffer, input_length, clear_default);

    view_dispatcher_switch_to_view(keyboard->view_dispatcher, JsKeyboardViewTextInput);

    view_dispatcher_run(keyboard->view_dispatcher);

    text_input_reset(keyboard->text_input);

    mjs_return(mjs, mjs_mk_string(mjs, buffer, ~0, true));
    free(buffer);
}

static void js_keyboard_byte(struct mjs* mjs) {
    JsKeyboardInst* keyboard = get_this_ctx(mjs);

    size_t input_length;
    if(!get_int_arg(mjs, 0, &input_length, true)) return;
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

    view_dispatcher_attach_to_gui(
        keyboard->view_dispatcher, furi_record_open(RECORD_GUI), ViewDispatcherTypeFullscreen);
    furi_record_close(RECORD_GUI);

    byte_input_set_result_callback(
        keyboard->byte_input, byte_input_callback, NULL, keyboard, buffer, input_length);

    view_dispatcher_switch_to_view(keyboard->view_dispatcher, JsKeyboardViewByteInput);

    view_dispatcher_run(keyboard->view_dispatcher);

    byte_input_set_result_callback(keyboard->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(keyboard->byte_input, "");

    mjs_return(mjs, mjs_mk_array_buf(mjs, (char*)buffer, input_length));
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
    keyboard->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(keyboard->view_dispatcher);
    view_dispatcher_add_view(
        keyboard->view_dispatcher,
        JsKeyboardViewTextInput,
        text_input_get_view(keyboard->text_input));
    view_dispatcher_add_view(
        keyboard->view_dispatcher,
        JsKeyboardViewByteInput,
        byte_input_get_view(keyboard->byte_input));
    *object = keyboard_obj;
    return keyboard;
}

static void js_keyboard_destroy(void* inst) {
    JsKeyboardInst* keyboard = inst;
    view_dispatcher_remove_view(keyboard->view_dispatcher, JsKeyboardViewByteInput);
    byte_input_free(keyboard->byte_input);
    view_dispatcher_remove_view(keyboard->view_dispatcher, JsKeyboardViewTextInput);
    text_input_free(keyboard->text_input);
    view_dispatcher_free(keyboard->view_dispatcher);
    free(keyboard->data);
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