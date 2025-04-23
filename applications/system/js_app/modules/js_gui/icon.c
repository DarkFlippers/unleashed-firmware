#include "../../js_modules.h"
#include <assets_icons.h>
#include <core/dangerous_defines.h>
#include <gui/icon_i.h>
#include <m-list.h>

typedef struct {
    const char* name;
    const Icon* data;
} IconDefinition;

#define ICON_DEF(icon)                   \
    (IconDefinition) {                   \
        .name = #icon, .data = &I_##icon \
    }

static const IconDefinition builtin_icons[] = {
    ICON_DEF(DolphinWait_59x54),
    ICON_DEF(js_script_10px),
};

// Firmware's Icon struct needs a frames array, and uses a small CompressHeader
// Here we use a variable size allocation to add the uncompressed data in same allocation
// Also use a one-long array pointing to later in the same struct as the frames array
// CompressHeader includes a first is_compressed byte so we don't need to compress (.fxbm is uncompressed)
typedef struct FURI_PACKED {
    Icon icon;
    uint8_t* frames[1];
    struct {
        uint8_t is_compressed;
        uint8_t uncompressed_data[];
    } frame;
} FxbmIconWrapper;

LIST_DEF(FxbmIconWrapperList, FxbmIconWrapper*, M_PTR_OPLIST); // NOLINT
#define M_OPL_FxbmIconWrapperList_t() LIST_OPLIST(FxbmIconWrapperList)

typedef struct {
    FxbmIconWrapperList_t fxbm_list;
} JsGuiIconInst;

static const JsValueDeclaration js_icon_get_arg_list[] = {
    JS_VALUE_SIMPLE(JsValueTypeString),
};
static const JsValueArguments js_icon_get_args = JS_VALUE_ARGS(js_icon_get_arg_list);

static void js_gui_icon_get_builtin(struct mjs* mjs) {
    const char* icon_name;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_icon_get_args, &icon_name);

    for(size_t i = 0; i < COUNT_OF(builtin_icons); i++) {
        if(strcmp(icon_name, builtin_icons[i].name) == 0) {
            mjs_return(mjs, mjs_mk_foreign(mjs, (void*)builtin_icons[i].data));
            return;
        }
    }

    JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "no such built-in icon");
}

static void js_gui_icon_load_fxbm(struct mjs* mjs) {
    const char* fxbm_path;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_icon_get_args, &fxbm_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    FxbmIconWrapper* fxbm = NULL;

    do {
        if(!storage_file_open(file, fxbm_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }

        struct {
            uint32_t size; // Total following size including width and height values
            uint32_t width;
            uint32_t height;
        } fxbm_header;
        if(storage_file_read(file, &fxbm_header, sizeof(fxbm_header)) != sizeof(fxbm_header)) {
            break;
        }

        size_t frame_size = fxbm_header.size - sizeof(uint32_t) * 2;
        fxbm = malloc(sizeof(FxbmIconWrapper) + frame_size);
        if(storage_file_read(file, fxbm->frame.uncompressed_data, frame_size) != frame_size) {
            free(fxbm);
            fxbm = NULL;
            break;
        }

        FURI_CONST_ASSIGN(fxbm->icon.width, fxbm_header.width);
        FURI_CONST_ASSIGN(fxbm->icon.height, fxbm_header.height);
        FURI_CONST_ASSIGN(fxbm->icon.frame_count, 1);
        FURI_CONST_ASSIGN(fxbm->icon.frame_rate, 1);
        FURI_CONST_ASSIGN_PTR(fxbm->icon.frames, fxbm->frames);
        fxbm->frames[0] = (void*)&fxbm->frame;
        fxbm->frame.is_compressed = false;
    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    if(!fxbm) {
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "could not load .fxbm icon");
    }

    JsGuiIconInst* js_icon = JS_GET_CONTEXT(mjs);
    FxbmIconWrapperList_push_back(js_icon->fxbm_list, fxbm);
    mjs_return(mjs, mjs_mk_foreign(mjs, (void*)&fxbm->icon));
}

static void* js_gui_icon_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    UNUSED(modules);
    JsGuiIconInst* js_icon = malloc(sizeof(JsGuiIconInst));
    FxbmIconWrapperList_init(js_icon->fxbm_list);
    *object = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, *object) {
        JS_FIELD(INST_PROP_NAME, mjs_mk_foreign(mjs, js_icon));
        JS_FIELD("getBuiltin", MJS_MK_FN(js_gui_icon_get_builtin));
        JS_FIELD("loadFxbm", MJS_MK_FN(js_gui_icon_load_fxbm));
    }
    return js_icon;
}

static void js_gui_icon_destroy(void* inst) {
    JsGuiIconInst* js_icon = inst;
    for
        M_EACH(fxbm, js_icon->fxbm_list, FxbmIconWrapperList_t) {
            free(*fxbm);
        }
    FxbmIconWrapperList_clear(js_icon->fxbm_list);
    free(js_icon);
}

static const JsModuleDescriptor js_gui_icon_desc = {
    "gui__icon",
    js_gui_icon_create,
    js_gui_icon_destroy,
    NULL,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_gui_icon_desc,
};

const FlipperAppPluginDescriptor* js_gui_icon_ep(void) {
    return &plugin_descriptor;
}
