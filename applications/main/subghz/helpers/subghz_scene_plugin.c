#include "subghz_scene_plugin.h"

#include "../subghz_i.h"
#include "subghz_feature_plugin.h"

static void subghz_scene_plugin_enter_stub(void* context, uint32_t index) {
    UNUSED(context);
    UNUSED(index);
}

static void subghz_scene_plugin_on_enter(SubGhz* subghz, const char* name) {
    char path[sizeof(SUBGHZ_FEATURE_PLUGIN_DIR) + 32];
    snprintf(path, sizeof(path), SUBGHZ_FEATURE_PLUGIN_DIR "subghz_%s.fal", name);

    subghz->scene_plugin = subghz_feature_plugin_load(
        subghz,
        &subghz->scene_plugin_manager,
        SUBGHZ_SCENE_PLUGIN_APP_ID,
        path,
        "Settings plugin\nis missing or\noutdated. Update\nresources.");
    if(subghz->scene_plugin) {
        subghz->scene_plugin->on_enter(subghz);
    }
}

static bool subghz_scene_plugin_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(!subghz->scene_plugin) {
        return subghz_feature_plugin_handle_missing(subghz, event);
    }
    return subghz->scene_plugin->on_event(subghz, event);
}

static void subghz_scene_plugin_on_exit(void* context) {
    SubGhz* subghz = context;
    if(subghz->scene_plugin) {
        subghz->scene_plugin->on_exit(subghz);
        subghz->scene_plugin = NULL;
        // Both outlive the scene's own reset and would point into the unmapped image: the list
        // keeps its enter callback (which cannot be NULL) and byte_input draws the header it was
        // last given by reference.
        variable_item_list_set_enter_callback(
            subghz->variable_item_list, subghz_scene_plugin_enter_stub, NULL);
        byte_input_set_header_text(subghz->byte_input, "");
    }
    subghz_feature_plugin_unload(subghz, &subghz->scene_plugin_manager);
}

// Scene subghz_scene_<name> forwarded to subghz_<name>.fal. Only on_enter differs per scene, so
// the scene table takes the shared event and exit handlers directly.
#define SUBGHZ_SCENE_PLUGIN(name)                                               \
    void subghz_scene_##name##_on_enter(void* context) {                        \
        subghz_scene_plugin_on_enter(context, #name);                           \
    }                                                                           \
    bool subghz_scene_##name##_on_event(void* context, SceneManagerEvent event) \
        __attribute__((alias("subghz_scene_plugin_on_event")));                 \
    void subghz_scene_##name##_on_exit(void* context)                           \
        __attribute__((alias("subghz_scene_plugin_on_exit")));

SUBGHZ_SCENE_PLUGIN(signal_settings)
SUBGHZ_SCENE_PLUGIN(radio_settings)
