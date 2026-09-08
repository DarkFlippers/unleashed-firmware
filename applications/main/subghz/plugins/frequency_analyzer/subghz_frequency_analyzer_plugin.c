#include "subghz_frequency_analyzer.h"

#include <subghz/subghz_i.h>
#include <subghz/helpers/subghz_feature_plugin.h>
#include <subghz/helpers/subghz_frequency_analyzer_plugin.h>

#include <flipper_application/flipper_application.h>

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

/** One analyzer at a time, and each scene entry maps a fresh copy of the plugin, so a file
 * static is enough. */
static SubGhzFrequencyAnalyzer* subghz_frequency_analyzer = NULL;

static void subghz_frequency_analyzer_plugin_callback(SubGhzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

static void subghz_frequency_analyzer_plugin_on_enter(SubGhz* subghz) {
    furi_assert(!subghz_frequency_analyzer);

    subghz_frequency_analyzer = subghz_frequency_analyzer_alloc(subghz->txrx);
    subghz_frequency_analyzer_set_callback(
        subghz_frequency_analyzer, subghz_frequency_analyzer_plugin_callback, subghz);
    subghz_frequency_analyzer_feedback_level(
        subghz_frequency_analyzer, subghz->last_settings->frequency_analyzer_feedback_level, true);

    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdFrequencyAnalyzer,
        subghz_frequency_analyzer_get_view(subghz_frequency_analyzer));
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdFrequencyAnalyzer);
}

static bool subghz_frequency_analyzer_plugin_on_event(SubGhz* subghz, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneAnalyzerLock) {
            notification_message(subghz->notifications, &sequence_set_green_255);
            switch(subghz_frequency_analyzer_feedback_level(
                subghz_frequency_analyzer, SubGHzFrequencyAnalyzerFeedbackLevelAll, false)) {
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
                subghz_frequency_analyzer_get_frequency_to_save(subghz_frequency_analyzer);
            if(frequency > 0) {
                subghz->last_settings->frequency = frequency;
                // Disable Hopping before opening the receiver scene!
                if(subghz->last_settings->enable_hopping) {
                    subghz->last_settings->enable_hopping = false;
                }
                subghz_last_settings_save(subghz->last_settings);
            }

            return true;
        }
    }
    return false;
}

static void subghz_frequency_analyzer_plugin_on_exit(SubGhz* subghz) {
    furi_assert(subghz_frequency_analyzer);

    notification_message(subghz->notifications, &sequence_reset_rgb);

    // Read before the view goes away: the trigger level lives in the worker, which the view frees
    // on exit.
    subghz->last_settings->frequency_analyzer_feedback_level =
        subghz_frequency_analyzer_feedback_level(subghz_frequency_analyzer, 0, false);
    subghz->last_settings->frequency_analyzer_trigger =
        subghz_frequency_analyzer_get_trigger_level(subghz_frequency_analyzer);
    subghz_last_settings_save(subghz->last_settings);

    // Park on an app-owned view before dropping ours. Removing the view while it is still the
    // current one leaves the dispatcher with no view, which stops the app's event loop for good
    // (view_dispatcher.c, view_dispatcher_set_current_view). The switch also runs our view's exit
    // handler, which joins the worker thread - that thread runs plugin code, so it has to be gone
    // before the caller unmaps us.
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdWidget);
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdFrequencyAnalyzer);
    subghz_frequency_analyzer_free(subghz_frequency_analyzer);
    subghz_frequency_analyzer = NULL;
}

static const SubGhzFrequencyAnalyzerPlugin subghz_frequency_analyzer_plugin = {
    .on_enter = subghz_frequency_analyzer_plugin_on_enter,
    .on_event = subghz_frequency_analyzer_plugin_on_event,
    .on_exit = subghz_frequency_analyzer_plugin_on_exit,
};

static const FlipperAppPluginDescriptor subghz_frequency_analyzer_plugin_descriptor = {
    .appid = SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_APP_ID,
    .ep_api_version = SUBGHZ_FEATURE_PLUGIN_API_VERSION,
    .entry_point = &subghz_frequency_analyzer_plugin,
};

const FlipperAppPluginDescriptor* subghz_frequency_analyzer_ep(void) {
    return &subghz_frequency_analyzer_plugin_descriptor;
}
