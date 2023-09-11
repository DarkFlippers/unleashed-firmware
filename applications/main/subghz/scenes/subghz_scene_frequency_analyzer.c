#include "../subghz_i.h"
#include "../views/subghz_frequency_analyzer.h"

#define TAG "SubGhzSceneFrequencyAnalyzer"

static const NotificationSequence sequence_saved = {
    &message_blink_stop,
    &message_blue_0,
    &message_green_255,
    &message_red_0,
    &message_vibro_on,
    &message_delay_100,
    &message_vibro_off,
    NULL,
};

void subghz_scene_frequency_analyzer_callback(SubGhzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

void subghz_scene_frequency_analyzer_on_enter(void* context) {
    SubGhz* subghz = context;
    subghz_frequency_analyzer_set_callback(
        subghz->subghz_frequency_analyzer, subghz_scene_frequency_analyzer_callback, subghz);
    subghz_frequency_analyzer_feedback_level(
        subghz->subghz_frequency_analyzer,
        subghz->last_settings->frequency_analyzer_feedback_level,
        true);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdFrequencyAnalyzer);
}

bool subghz_scene_frequency_analyzer_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneAnalyzerLock) {
            notification_message(subghz->notifications, &sequence_set_green_255);
            switch(subghz_frequency_analyzer_feedback_level(
                subghz->subghz_frequency_analyzer,
                SubGHzFrequencyAnalyzerFeedbackLevelAll,
                false)) {
            case SubGHzFrequencyAnalyzerFeedbackLevelAll:
                notification_message(subghz->notifications, &sequence_success);
                break;
            case SubGHzFrequencyAnalyzerFeedbackLevelVibro:
                notification_message(subghz->notifications, &sequence_single_vibro);
                break;
            case SubGHzFrequencyAnalyzerFeedbackLevelMute:
                break;
            }
            notification_message(subghz->notifications, &sequence_display_backlight_on);
            return true;
        } else if(event.event == SubGhzCustomEventSceneAnalyzerUnlock) {
            notification_message(subghz->notifications, &sequence_reset_rgb);
            return true;
        } else if(event.event == SubGhzCustomEventViewFreqAnalOkShort) {
            notification_message(subghz->notifications, &sequence_saved);
            uint32_t frequency =
                subghz_frequency_analyzer_get_frequency_to_save(subghz->subghz_frequency_analyzer);
            if(frequency > 0) {
                subghz->last_settings->frequency = frequency;
#ifdef FURI_DEBUG
                subghz_last_settings_log(subghz->last_settings);
#endif
                subghz_last_settings_save(subghz->last_settings);
            }

            return true;
        } else if(event.event == SubGhzCustomEventViewFreqAnalOkLong) {
            // Don't need to save, we already saved on short event
#ifdef FURI_DEBUG
            FURI_LOG_W(TAG, "Goto next scene!");
#endif
            //scene_manager_set_scene_state(subghz->scene_manager, SubGhzSceneStart, 10);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiver);
            return true;
        }
    }
    return false;
}

void subghz_scene_frequency_analyzer_on_exit(void* context) {
    SubGhz* subghz = context;
    notification_message(subghz->notifications, &sequence_reset_rgb);

    subghz->last_settings->frequency_analyzer_feedback_level =
        subghz_frequency_analyzer_feedback_level(subghz->subghz_frequency_analyzer, 0, false);
    subghz->last_settings->frequency_analyzer_trigger =
        subghz_frequency_analyzer_get_trigger_level(subghz->subghz_frequency_analyzer);
    subghz_last_settings_save(subghz->last_settings);
}
