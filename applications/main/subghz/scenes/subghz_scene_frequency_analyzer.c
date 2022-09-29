#include "../subghz_i.h"
#include <dolphin/dolphin.h>

#define TAG "SubGhzSceneFrequencyAnalyzer"

void subghz_scene_frequency_analyzer_callback(SubGhzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

void subghz_scene_frequency_analyzer_on_enter(void* context) {
    SubGhz* subghz = context;
    DOLPHIN_DEED(DolphinDeedSubGhzFrequencyAnalyzer);
    subghz_frequency_analyzer_set_callback(
        subghz->subghz_frequency_analyzer, subghz_scene_frequency_analyzer_callback, subghz);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdFrequencyAnalyzer);
}

bool subghz_scene_frequency_analyzer_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventViewReceiverOK) {
            uint32_t frequency =
                subghz_frequency_analyzer_get_frequency_to_save(subghz->subghz_frequency_analyzer);
            if(frequency > 0) {
                subghz->last_settings->frequency = frequency;
                subghz_last_settings_save(subghz->last_settings);
            }

            return true;
        } else if(event.event == SubGhzCustomEventViewReceiverUnlock) {
            // Don't need to save, we already saved on short event
#if FURI_DEBUG
            FURI_LOG_W(TAG, "Goto next scene!");
#endif
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiver);
            return true;
        }
    }
    return false;
}

void subghz_scene_frequency_analyzer_on_exit(void* context) {
    SubGhz* subghz = context;
    notification_message(subghz->notifications, &sequence_reset_rgb);
}
