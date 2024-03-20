#include <gui/modules/submenu.h>
#include <gui/view_dispatcher.h>
#include <gui/view.h>
#include "../js_modules.h"

typedef struct {
    Submenu* submenu;
    ViewDispatcher* view_dispatcher;
    uint32_t result;
    bool accepted;
} JsSubmenuInst;

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

static void submenu_callback(void* context, uint32_t id) {
    JsSubmenuInst* submenu = context;
    submenu->result = id;
    submenu->accepted = true;
    view_dispatcher_stop(submenu->view_dispatcher);
}

static bool submenu_exit(void* context) {
    JsSubmenuInst* submenu = context;
    submenu->result = 0;
    submenu->accepted = false;
    view_dispatcher_stop(submenu->view_dispatcher);
    return true;
}

static void js_submenu_add_item(struct mjs* mjs) {
    JsSubmenuInst* submenu = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 2)) return;

    mjs_val_t label_arg = mjs_arg(mjs, 0);
    const char* label = mjs_get_string(mjs, &label_arg, NULL);
    if(!label) {
        ret_bad_args(mjs, "Label must be a string");
        return;
    }

    mjs_val_t id_arg = mjs_arg(mjs, 1);
    if(!mjs_is_number(id_arg)) {
        ret_bad_args(mjs, "Id must be a number");
        return;
    }
    int32_t id = mjs_get_int32(mjs, id_arg);

    submenu_add_item(submenu->submenu, label, id, submenu_callback, submenu);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_submenu_set_header(struct mjs* mjs) {
    JsSubmenuInst* submenu = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 1)) return;

    mjs_val_t header_arg = mjs_arg(mjs, 0);
    const char* header = mjs_get_string(mjs, &header_arg, NULL);
    if(!header) {
        ret_bad_args(mjs, "Header must be a string");
        return;
    }

    submenu_set_header(submenu->submenu, header);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_submenu_show(struct mjs* mjs) {
    JsSubmenuInst* submenu = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;

    Gui* gui = furi_record_open(RECORD_GUI);
    submenu->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(submenu->view_dispatcher);
    view_dispatcher_add_view(submenu->view_dispatcher, 0, submenu_get_view(submenu->submenu));
    view_dispatcher_set_event_callback_context(submenu->view_dispatcher, submenu);
    view_dispatcher_set_navigation_event_callback(submenu->view_dispatcher, submenu_exit);
    view_dispatcher_attach_to_gui(submenu->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(submenu->view_dispatcher, 0);

    view_dispatcher_run(submenu->view_dispatcher);

    view_dispatcher_remove_view(submenu->view_dispatcher, 0);
    view_dispatcher_free(submenu->view_dispatcher);
    submenu->view_dispatcher = NULL;
    furi_record_close(RECORD_GUI);

    submenu_reset(submenu->submenu);
    if(submenu->accepted) {
        mjs_return(mjs, mjs_mk_number(mjs, submenu->result));
    } else {
        mjs_return(mjs, MJS_UNDEFINED);
    }
}

static void* js_submenu_create(struct mjs* mjs, mjs_val_t* object) {
    JsSubmenuInst* submenu = malloc(sizeof(JsSubmenuInst));
    mjs_val_t submenu_obj = mjs_mk_object(mjs);
    mjs_set(mjs, submenu_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, submenu));
    mjs_set(mjs, submenu_obj, "addItem", ~0, MJS_MK_FN(js_submenu_add_item));
    mjs_set(mjs, submenu_obj, "setHeader", ~0, MJS_MK_FN(js_submenu_set_header));
    mjs_set(mjs, submenu_obj, "show", ~0, MJS_MK_FN(js_submenu_show));
    submenu->submenu = submenu_alloc();
    *object = submenu_obj;
    return submenu;
}

static void js_submenu_destroy(void* inst) {
    JsSubmenuInst* submenu = inst;
    submenu_free(submenu->submenu);
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