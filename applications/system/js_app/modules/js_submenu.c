#include <gui/modules/submenu.h>
#include <gui/view_dispatcher.h>
#include <gui/view.h>
#include "../js_modules.h"

typedef struct {
    Submenu* submenu;
    ViewDispatcher* view_dispatcher;
    uint32_t result;
} JsSubmenuInst;

typedef enum {
    JsSubmenuViewSubmenu,
} JsSubmenuView;

static JsSubmenuInst* get_this_ctx(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubmenuInst* storage = mjs_get_ptr(mjs, obj_inst);
    furi_assert(storage);
    return storage;
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

static bool get_str_arg(struct mjs* mjs, size_t index, const char** value) {
    mjs_val_t str_obj = mjs_arg(mjs, index);
    if(!mjs_is_string(str_obj)) {
        ret_bad_args(mjs, "Argument must be a string");
        return false;
    }
    size_t str_len = 0;
    *value = mjs_get_string(mjs, &str_obj, &str_len);
    if((str_len == 0) || (*value == NULL)) {
        ret_bad_args(mjs, "Bad string argument");
        return false;
    }
    return true;
}

static int32_t get_int_arg(struct mjs* mjs, size_t index, int32_t* value) {
    mjs_val_t int_obj = mjs_arg(mjs, index);
    if(!mjs_is_number(int_obj)) {
        ret_bad_args(mjs, "Argument must be a number");
        return false;
    }
    *value = mjs_get_int32(mjs, int_obj);
    return true;
}

static void submenu_callback(void* context, uint32_t id) {
    UNUSED(id);
    JsSubmenuInst* submenu = context;
    submenu->result = id;
    view_dispatcher_stop(submenu->view_dispatcher);
}

static void js_submenu_add_item(struct mjs* mjs) {
    JsSubmenuInst* submenu = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 2)) return;

    const char* label;
    if(!get_str_arg(mjs, 0, &label)) return;

    int32_t id;
    if(!get_int_arg(mjs, 1, &id)) return;

    submenu_add_item(submenu->submenu, label, id, submenu_callback, submenu);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_submenu_set_header(struct mjs* mjs) {
    JsSubmenuInst* submenu = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 1)) return;

    const char* header;
    if(!get_str_arg(mjs, 0, &header)) return;

    submenu_set_header(submenu->submenu, header);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_submenu_show(struct mjs* mjs) {
    JsSubmenuInst* submenu = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;
    submenu->result = 0;

    view_dispatcher_attach_to_gui(
        submenu->view_dispatcher, furi_record_open(RECORD_GUI), ViewDispatcherTypeFullscreen);
    furi_record_close(RECORD_GUI);

    view_dispatcher_switch_to_view(submenu->view_dispatcher, JsSubmenuViewSubmenu);

    view_dispatcher_run(submenu->view_dispatcher);

    submenu_reset(submenu->submenu);

    mjs_return(mjs, mjs_mk_number(mjs, submenu->result));
}

static void* js_submenu_create(struct mjs* mjs, mjs_val_t* object) {
    JsSubmenuInst* submenu = malloc(sizeof(JsSubmenuInst));
    mjs_val_t submenu_obj = mjs_mk_object(mjs);
    mjs_set(mjs, submenu_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, submenu));
    mjs_set(mjs, submenu_obj, "addItem", ~0, MJS_MK_FN(js_submenu_add_item));
    mjs_set(mjs, submenu_obj, "setHeader", ~0, MJS_MK_FN(js_submenu_set_header));
    mjs_set(mjs, submenu_obj, "show", ~0, MJS_MK_FN(js_submenu_show));
    submenu->submenu = submenu_alloc();
    submenu->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(submenu->view_dispatcher);
    view_dispatcher_add_view(
        submenu->view_dispatcher, JsSubmenuViewSubmenu, submenu_get_view(submenu->submenu));
    *object = submenu_obj;
    return submenu;
}

static void js_submenu_destroy(void* inst) {
    JsSubmenuInst* submenu = inst;
    view_dispatcher_remove_view(submenu->view_dispatcher, JsSubmenuViewSubmenu);
    submenu_free(submenu->submenu);
    view_dispatcher_free(submenu->view_dispatcher);
    free(submenu);
}

static const JsModuleDescriptor js_submenu_desc = {
    "submenu",
    js_submenu_create,
    js_submenu_destroy,
};

static const FlipperAppPluginDescriptor submenu_plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_submenu_desc,
};

const FlipperAppPluginDescriptor* js_submenu_ep(void) {
    return &submenu_plugin_descriptor;
}