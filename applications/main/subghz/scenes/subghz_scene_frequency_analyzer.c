#include "../subghz_i.h" // IWYU pragma: keep
#include "../helpers/subghz_frequency_analyzer_plugin.h"

void subghz_scene_frequency_analyzer_on_enter(void* context) {
    SubGhz* subghz = context;

    if(!subghz_frequency_analyzer_plugin_load(subghz)) {
        furi_string_set(
            subghz->error_str, "Analyzer plugin\nis missing or\noutdated. Update\nresources.");
        // Reported from the event loop: switching scenes inside on_enter would run this scene's
        // own on_exit before it had finished entering.
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneAnalyzerMissing);
    }
}

bool subghz_scene_frequency_analyzer_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneAnalyzerMissing) {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            return true;
        } else if(event.event == SubGhzCustomEventViewFreqAnalOkLong) {
            // Leaving the scene stays here rather than in the plugin: it runs this scene's
            // on_exit, which unmaps the image the plugin would return into.
            // Don't need to save, we already saved on short event (and on exit event too)
            subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);
            scene_manager_previous_scene(subghz->scene_manager); // Stops the worker
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiver);
            return true;
        }
    }

    if(!subghz->freq_analyzer_plugin) {
        return false;
    }
    return subghz->freq_analyzer_plugin->on_event(subghz, event);
}

void subghz_scene_frequency_analyzer_on_exit(void* context) {
    SubGhz* subghz = context;
    subghz_frequency_analyzer_plugin_unload(subghz);
}
