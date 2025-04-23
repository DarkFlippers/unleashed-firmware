#include <core/common_defines.h>
#include "js_modules.h"
#include <m-array.h>
#include <dialogs/dialogs.h>
#include <assets_icons.h>

#include "modules/js_flipper.h"
#ifdef FW_CFG_unit_tests
#include "modules/js_tests.h"
#endif

#define TAG "JS modules"

// Absolute path is used to make possible plugin load from CLI
#define MODULES_PATH "/ext/apps_data/js_app/plugins"

typedef struct {
    FuriString* name;
    const JsModuleConstructor create;
    const JsModuleDestructor destroy;
    void* context;
} JsModuleData;

// not using:
//   - a dict because ordering is required
//   - a bptree because it forces a sorted ordering
//   - an rbtree because i deemed it more tedious to implement, and with the
//     amount of modules in use (under 10 in the overwhelming majority of cases)
//     i bet it's going to be slower than a plain array
ARRAY_DEF(JsModuleArray, JsModuleData, M_POD_OPLIST);
#define M_OPL_JsModuleArray_t() ARRAY_OPLIST(JsModuleArray)

static const JsModuleDescriptor modules_builtin[] = {
    {"flipper", js_flipper_create, NULL, NULL},
#ifdef FW_CFG_unit_tests
    {"tests", js_tests_create, NULL, NULL},
#endif
};

struct JsModules {
    struct mjs* mjs;
    JsModuleArray_t modules;
    PluginManager* plugin_manager;
    CompositeApiResolver* resolver;
};

JsModules* js_modules_create(struct mjs* mjs, CompositeApiResolver* resolver) {
    JsModules* modules = malloc(sizeof(JsModules));
    modules->mjs = mjs;
    JsModuleArray_init(modules->modules);

    modules->plugin_manager = plugin_manager_alloc(
        PLUGIN_APP_ID, PLUGIN_API_VERSION, composite_api_resolver_get(resolver));

    modules->resolver = resolver;

    return modules;
}

void js_modules_destroy(JsModules* instance) {
    for
        M_EACH(module, instance->modules, JsModuleArray_t) {
            FURI_LOG_T(TAG, "Tearing down %s", furi_string_get_cstr(module->name));
            if(module->destroy) module->destroy(module->context);
            furi_string_free(module->name);
        }
    plugin_manager_free(instance->plugin_manager);
    JsModuleArray_clear(instance->modules);
    free(instance);
}

JsModuleData* js_find_loaded_module(JsModules* instance, const char* name) {
    for
        M_EACH(module, instance->modules, JsModuleArray_t) {
            if(furi_string_cmp_str(module->name, name) == 0) return module;
        }
    return NULL;
}

mjs_val_t js_module_require(JsModules* modules, const char* name, size_t name_len) {
    // Ignore the initial part of the module name
    const char* optional_module_prefix = "@" JS_SDK_VENDOR "/fz-sdk/";
    if(strncmp(name, optional_module_prefix, strlen(optional_module_prefix)) == 0) {
        name += strlen(optional_module_prefix);
    }

    // Check if module is already installed
    JsModuleData* module_inst = js_find_loaded_module(modules, name);
    if(module_inst) { //-V547
        mjs_prepend_errorf(
            modules->mjs, MJS_BAD_ARGS_ERROR, "\"%s\" module is already installed", name);
        return MJS_UNDEFINED;
    }

    bool module_found = false;
    // Check built-in modules
    for(size_t i = 0; i < COUNT_OF(modules_builtin); i++) { //-V1008
        size_t name_compare_len = strlen(modules_builtin[i].name);

        if(name_compare_len != name_len) {
            continue;
        }

        if(strncmp(name, modules_builtin[i].name, name_compare_len) == 0) {
            JsModuleData module = {
                .create = modules_builtin[i].create,
                .destroy = modules_builtin[i].destroy,
                .name = furi_string_alloc_set_str(name),
            };
            JsModuleArray_push_at(modules->modules, 0, module);
            module_found = true;
            FURI_LOG_I(TAG, "Using built-in module %s", name);
            break;
        }
    }

    // External module load
    if(!module_found) {
        FuriString* deslashed_name = furi_string_alloc_set_str(name);
        furi_string_replace_all_str(deslashed_name, "/", "__");
        FuriString* module_path = furi_string_alloc();
        furi_string_printf(
            module_path, "%s/js_%s.fal", MODULES_PATH, furi_string_get_cstr(deslashed_name));
        FURI_LOG_I(
            TAG, "Loading external module %s from %s", name, furi_string_get_cstr(module_path));
        do {
            uint32_t plugin_cnt_last = plugin_manager_get_count(modules->plugin_manager);
            PluginManagerError load_error = plugin_manager_load_single(
                modules->plugin_manager, furi_string_get_cstr(module_path));
            if(load_error != PluginManagerErrorNone) {
                FURI_LOG_E(
                    TAG,
                    "Module %s load error. It may depend on other modules that are not yet loaded.",
                    name);
                break;
            }
            const JsModuleDescriptor* plugin =
                plugin_manager_get_ep(modules->plugin_manager, plugin_cnt_last);
            furi_assert(plugin);

            if(furi_string_cmp_str(deslashed_name, plugin->name) != 0) {
                FURI_LOG_E(TAG, "Module name mismatch %s", plugin->name);
                break;
            }
            JsModuleData module = {
                .create = plugin->create,
                .destroy = plugin->destroy,
                .name = furi_string_alloc_set_str(name),
            };
            JsModuleArray_push_at(modules->modules, 0, module);

            if(plugin->api_interface) {
                FURI_LOG_I(TAG, "Added module API to composite resolver: %s", plugin->name);
                composite_api_resolver_add(modules->resolver, plugin->api_interface);
            }

            module_found = true;
        } while(0);
        furi_string_free(module_path);
        furi_string_free(deslashed_name);
    }

    // Run module constructor
    mjs_val_t module_object = MJS_UNDEFINED;
    if(module_found) {
        module_inst = js_find_loaded_module(modules, name);
        furi_assert(module_inst);
        if(module_inst->create) { //-V779
            module_inst->context = module_inst->create(modules->mjs, &module_object, modules);
        }
    }

    if(module_object == MJS_UNDEFINED) { //-V547
        mjs_prepend_errorf(modules->mjs, MJS_BAD_ARGS_ERROR, "\"%s\" module load fail", name);
    }

    return module_object;
}

void* js_module_get(JsModules* modules, const char* name) {
    FuriString* module_name = furi_string_alloc_set_str(name);
    JsModuleData* module_inst = js_find_loaded_module(modules, name);
    furi_string_free(module_name);
    return module_inst ? module_inst->context : NULL;
}

typedef enum {
    JsSdkCompatStatusCompatible,
    JsSdkCompatStatusFirmwareTooOld,
    JsSdkCompatStatusFirmwareTooNew,
} JsSdkCompatStatus;

/**
 * @brief Checks compatibility between the firmware and the JS SDK version
 *        expected by the script
 */
static JsSdkCompatStatus
    js_internal_sdk_compatibility_status(int32_t exp_major, int32_t exp_minor) {
    if(exp_major < JS_SDK_MAJOR) return JsSdkCompatStatusFirmwareTooNew;
    if(exp_major > JS_SDK_MAJOR || exp_minor > JS_SDK_MINOR)
        return JsSdkCompatStatusFirmwareTooOld;
    return JsSdkCompatStatusCompatible;
}

static const JsValueDeclaration js_sdk_version_arg_list[] = {
    JS_VALUE_SIMPLE(JsValueTypeInt32),
    JS_VALUE_SIMPLE(JsValueTypeInt32),
};
static const JsValueArguments js_sdk_version_args = JS_VALUE_ARGS(js_sdk_version_arg_list);

void js_sdk_compatibility_status(struct mjs* mjs) {
    int32_t major, minor;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_sdk_version_args, &major, &minor);
    JsSdkCompatStatus status = js_internal_sdk_compatibility_status(major, minor);
    switch(status) {
    case JsSdkCompatStatusCompatible:
        mjs_return(mjs, mjs_mk_string(mjs, "compatible", ~0, 0));
        return;
    case JsSdkCompatStatusFirmwareTooOld:
        mjs_return(mjs, mjs_mk_string(mjs, "firmwareTooOld", ~0, 0));
        return;
    case JsSdkCompatStatusFirmwareTooNew:
        mjs_return(mjs, mjs_mk_string(mjs, "firmwareTooNew", ~0, 0));
        return;
    }
}

void js_is_sdk_compatible(struct mjs* mjs) {
    int32_t major, minor;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_sdk_version_args, &major, &minor);
    JsSdkCompatStatus status = js_internal_sdk_compatibility_status(major, minor);
    mjs_return(mjs, mjs_mk_boolean(mjs, status == JsSdkCompatStatusCompatible));
}

/**
 * @brief Asks the user whether to continue executing an incompatible script
 */
static bool js_internal_compat_ask_user(const char* message) {
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* dialog = dialog_message_alloc();
    dialog_message_set_header(dialog, message, 64, 0, AlignCenter, AlignTop);
    dialog_message_set_text(
        dialog, "This script may not\nwork as expected", 79, 32, AlignCenter, AlignCenter);
    dialog_message_set_icon(dialog, &I_Warning_30x23, 0, 18);
    dialog_message_set_buttons(dialog, "Go back", NULL, "Run anyway");
    DialogMessageButton choice = dialog_message_show(dialogs, dialog);
    dialog_message_free(dialog);
    furi_record_close(RECORD_DIALOGS);
    return choice == DialogMessageButtonRight;
}

void js_check_sdk_compatibility(struct mjs* mjs) {
    int32_t major, minor;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_sdk_version_args, &major, &minor);
    JsSdkCompatStatus status = js_internal_sdk_compatibility_status(major, minor);
    if(status != JsSdkCompatStatusCompatible) {
        FURI_LOG_E(
            TAG,
            "Script requests JS SDK %ld.%ld, firmware provides JS SDK %d.%d",
            major,
            minor,
            JS_SDK_MAJOR,
            JS_SDK_MINOR);

        const char* message = (status == JsSdkCompatStatusFirmwareTooOld) ? "Outdated Firmware" :
                                                                            "Outdated Script";
        if(!js_internal_compat_ask_user(message)) {
            JS_ERROR_AND_RETURN(mjs, MJS_NOT_IMPLEMENTED_ERROR, "Incompatible script");
        }
    }
}

static const char* extra_features[] = {
    "baseline", // dummy "feature"
    "gpio-pwm",
    "gui-widget",
    "serial-framing",
    "gui-widget-extras",

    // extra modules
    "blebeacon",
    "i2c",
    "spi",
    "subghz",
    "usbdisk",
    "vgm",
};

/**
 * @brief Determines whether a feature is supported
 */
static bool js_internal_supports(const char* feature) {
    for(size_t i = 0; i < COUNT_OF(extra_features); i++) { // -V1008
        if(strcmp(feature, extra_features[i]) == 0) return true;
    }
    return false;
}

/**
 * @brief Determines whether all of the requested features are supported
 */
static bool js_internal_supports_all_of(struct mjs* mjs, mjs_val_t feature_arr) {
    furi_assert(mjs_is_array(feature_arr));

    for(size_t i = 0; i < mjs_array_length(mjs, feature_arr); i++) {
        mjs_val_t feature = mjs_array_get(mjs, feature_arr, i);
        const char* feature_str = mjs_get_string(mjs, &feature, NULL);
        if(!feature_str) return false;

        if(!js_internal_supports(feature_str)) return false;
    }

    return true;
}

static const JsValueDeclaration js_sdk_features_arg_list[] = {
    JS_VALUE_SIMPLE(JsValueTypeAnyArray),
};
static const JsValueArguments js_sdk_features_args = JS_VALUE_ARGS(js_sdk_features_arg_list);

void js_does_sdk_support(struct mjs* mjs) {
    mjs_val_t features;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_sdk_features_args, &features);
    mjs_return(mjs, mjs_mk_boolean(mjs, js_internal_supports_all_of(mjs, features)));
}

void js_check_sdk_features(struct mjs* mjs) {
    mjs_val_t features;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_sdk_features_args, &features);
    if(!js_internal_supports_all_of(mjs, features)) {
        FURI_LOG_E(TAG, "Script requests unsupported features");

        if(!js_internal_compat_ask_user("Unsupported Feature")) {
            JS_ERROR_AND_RETURN(mjs, MJS_NOT_IMPLEMENTED_ERROR, "Incompatible script");
        }
    }
}
