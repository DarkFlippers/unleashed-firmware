#include <gui/modules/text_box.h>
#include <gui/view_holder.h>
#include "../js_modules.h"

typedef struct {
    TextBox* text_box;
    ViewHolder* view_holder;
    FuriString* text;
    bool is_shown;
} JsTextboxInst;

static JsTextboxInst* get_this_ctx(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsTextboxInst* textbox = mjs_get_ptr(mjs, obj_inst);
    furi_assert(textbox);
    return textbox;
}

static void ret_bad_args(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "%s", error);
    mjs_return(mjs, MJS_UNDEFINED);
}

static bool check_arg_count(struct mjs* mjs, size_t count) {
    size_t num_args = mjs_nargs(mjs);
    if(num_args != count) {
        ret_bad_args(mjs, "Wrong argument count");
        return false;
    }
    return true;
}

static void js_textbox_set_config(struct mjs* mjs) {
    JsTextboxInst* textbox = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 2)) return;

    TextBoxFocus set_focus = TextBoxFocusStart;
    mjs_val_t focus_arg = mjs_arg(mjs, 0);
    const char* focus = mjs_get_string(mjs, &focus_arg, NULL);
    if(!focus) {
        ret_bad_args(mjs, "Focus must be a string");
        return;
    } else {
        if(!strncmp(focus, "start", strlen("start"))) {
            set_focus = TextBoxFocusStart;
        } else if(!strncmp(focus, "end", strlen("end"))) {
            set_focus = TextBoxFocusEnd;
        } else {
            ret_bad_args(mjs, "Bad focus value");
            return;
        }
    }

    TextBoxFont set_font = TextBoxFontText;
    mjs_val_t font_arg = mjs_arg(mjs, 1);
    const char* font = mjs_get_string(mjs, &font_arg, NULL);
    if(!font) {
        ret_bad_args(mjs, "Font must be a string");
        return;
    } else {
        if(!strncmp(font, "text", strlen("text"))) {
            set_font = TextBoxFontText;
        } else if(!strncmp(font, "hex", strlen("hex"))) {
            set_font = TextBoxFontHex;
        } else {
            ret_bad_args(mjs, "Bad font value");
            return;
        }
    }

    text_box_set_focus(textbox->text_box, set_focus);
    text_box_set_font(textbox->text_box, set_font);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_textbox_add_text(struct mjs* mjs) {
    JsTextboxInst* textbox = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 1)) return;

    mjs_val_t text_arg = mjs_arg(mjs, 0);
    size_t text_len = 0;
    const char* text = mjs_get_string(mjs, &text_arg, &text_len);
    if(!text) {
        ret_bad_args(mjs, "Text must be a string");
        return;
    }

    // Avoid condition race between GUI and JS thread
    text_box_set_text(textbox->text_box, "");

    size_t new_len = furi_string_size(textbox->text) + text_len;
    if(new_len >= 4096) {
        furi_string_right(textbox->text, new_len / 2);
    }

    furi_string_cat(textbox->text, text);

    text_box_set_text(textbox->text_box, furi_string_get_cstr(textbox->text));

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_textbox_clear_text(struct mjs* mjs) {
    JsTextboxInst* textbox = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;

    // Avoid condition race between GUI and JS thread
    text_box_set_text(textbox->text_box, "");

    furi_string_reset(textbox->text);

    text_box_set_text(textbox->text_box, furi_string_get_cstr(textbox->text));

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_textbox_is_open(struct mjs* mjs) {
    JsTextboxInst* textbox = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;

    mjs_return(mjs, mjs_mk_boolean(mjs, textbox->is_shown));
}

static void textbox_callback(void* context, uint32_t arg) {
    UNUSED(arg);
    JsTextboxInst* textbox = context;
    view_holder_stop(textbox->view_holder);
    textbox->is_shown = false;
}

static void textbox_exit(void* context) {
    JsTextboxInst* textbox = context;
    // Using timer to schedule view_holder stop, will not work under high CPU load
    furi_timer_pending_callback(textbox_callback, textbox, 0);
}

static void js_textbox_show(struct mjs* mjs) {
    JsTextboxInst* textbox = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;

    if(textbox->is_shown) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Textbox is already shown");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    view_holder_start(textbox->view_holder);
    textbox->is_shown = true;

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_textbox_close(struct mjs* mjs) {
    JsTextboxInst* textbox = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;

    view_holder_stop(textbox->view_holder);
    textbox->is_shown = false;

    mjs_return(mjs, MJS_UNDEFINED);
}

static void* js_textbox_create(struct mjs* mjs, mjs_val_t* object) {
    JsTextboxInst* textbox = malloc(sizeof(JsTextboxInst));

    mjs_val_t textbox_obj = mjs_mk_object(mjs);
    mjs_set(mjs, textbox_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, textbox));
    mjs_set(mjs, textbox_obj, "setConfig", ~0, MJS_MK_FN(js_textbox_set_config));
    mjs_set(mjs, textbox_obj, "addText", ~0, MJS_MK_FN(js_textbox_add_text));
    mjs_set(mjs, textbox_obj, "clearText", ~0, MJS_MK_FN(js_textbox_clear_text));
    mjs_set(mjs, textbox_obj, "isOpen", ~0, MJS_MK_FN(js_textbox_is_open));
    mjs_set(mjs, textbox_obj, "show", ~0, MJS_MK_FN(js_textbox_show));
    mjs_set(mjs, textbox_obj, "close", ~0, MJS_MK_FN(js_textbox_close));

    textbox->text = furi_string_alloc();
    textbox->text_box = text_box_alloc();

    Gui* gui = furi_record_open(RECORD_GUI);
    textbox->view_holder = view_holder_alloc();
    view_holder_attach_to_gui(textbox->view_holder, gui);
    view_holder_set_back_callback(textbox->view_holder, textbox_exit, textbox);
    view_holder_set_view(textbox->view_holder, text_box_get_view(textbox->text_box));

    *object = textbox_obj;
    return textbox;
}

static void js_textbox_destroy(void* inst) {
    JsTextboxInst* textbox = inst;

    view_holder_stop(textbox->view_holder);
    view_holder_free(textbox->view_holder);
    textbox->view_holder = NULL;

    furi_record_close(RECORD_GUI);

    text_box_reset(textbox->text_box);
    furi_string_reset(textbox->text);

    text_box_free(textbox->text_box);
    furi_string_free(textbox->text);
    free(textbox);
}

static const JsModuleDescriptor js_textbox_desc = {
    "textbox",
    js_textbox_create,
    js_textbox_destroy,
};

static const FlipperAppPluginDescriptor textbox_plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_textbox_desc,
};

const FlipperAppPluginDescriptor* js_textbox_ep(void) {
    return &textbox_plugin_descriptor;
}
