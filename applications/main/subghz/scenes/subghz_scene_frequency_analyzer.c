#include "../subghz_i.h" // IWYU pragma: keep
#include "../helpers/subghz_feature_plugin.h"
#include "../helpers/subghz_frequency_analyzer_plugin.h"

void subghz_scene_frequency_analyzer_on_enter(void* context) {
    subghz_frequency_analyzer_plugin_load(context);
}

bool subghz_scene_frequency_analyzer_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(!subghz->freq_analyzer_plugin) {
        return subghz_feature_plugin_handle_missing(subghz, event);
    }

    if(event.type == SceneManagerEventTypeCustom &&
       event.event == SubGhzCustomEventViewFreqAnalOkLong) {
        // Leaving the scene stays here rather than in the plugin: it runs this scene's on_exit,
        // which unmaps the image the plugin would return into.
        // Don't need to save, we already saved on short event (and on exit event too)
        subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);
        scene_manager_previous_scene(subghz->scene_manager); // Stops the worker
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiver);
        return true;
    }

    return subghz->freq_analyzer_plugin->on_event(subghz, event);
}

void subghz_scene_frequency_analyzer_on_exit(void* context) {
    SubGhz* subghz = context;
    subghz_frequency_analyzer_plugin_unload(subghz);
}
