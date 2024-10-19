#include "../../js_modules.h"
#include <dialogs/dialogs.h>

// File icon
#include <gui/icon_i.h>
static const uint8_t _I_file_10px_0[] = {
    0x00, 0x7f, 0x00, 0xa1, 0x00, 0x2d, 0x01, 0xe1, 0x01, 0x0d, 0x01,
    0x01, 0x01, 0x7d, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0x01,
};
static const uint8_t* const _I_file_10px[] = {_I_file_10px_0};

static const Icon I_file_10px =
    {.width = 10, .height = 10, .frame_count = 1, .frame_rate = 0, .frames = _I_file_10px};
// File icon end

static void js_gui_file_picker_pick_file(struct mjs* mjs) {
    const char *base_path, *extension;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&base_path), JS_ARG_STR(&extension));

    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    const DialogsFileBrowserOptions browser_options = {
        .extension = extension,
        .icon = &I_file_10px,
        .base_path = base_path,
    };
    FuriString* path = furi_string_alloc_set(base_path);
    if(dialog_file_browser_show(dialogs, path, path, &browser_options)) {
        mjs_return(mjs, mjs_mk_string(mjs, furi_string_get_cstr(path), ~0, true));
    } else {
        mjs_return(mjs, MJS_UNDEFINED);
    }
    furi_string_free(path);
    furi_record_close(RECORD_DIALOGS);
}

static void* js_gui_file_picker_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    UNUSED(modules);
    *object = mjs_mk_object(mjs);
    mjs_set(mjs, *object, "pickFile", ~0, MJS_MK_FN(js_gui_file_picker_pick_file));
    return NULL;
}

static const JsModuleDescriptor js_gui_file_picker_desc = {
    "gui__file_picker",
    js_gui_file_picker_create,
    NULL,
    NULL,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_gui_file_picker_desc,
};

const FlipperAppPluginDescriptor* js_gui_file_picker_ep(void) {
    return &plugin_descriptor;
}
