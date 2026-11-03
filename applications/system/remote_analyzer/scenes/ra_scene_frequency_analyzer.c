#include "../remote_analyzer_i.h"

/** Short flash plus a nudge, for the moment a frequency is taken. */
static const NotificationSequence ra_sequence_picked = {
    &message_blink_stop,
    &message_blue_0,
    &message_green_255,
    &message_red_0,
    &message_vibro_on,
    &message_delay_100,
    &message_vibro_off,
    NULL,
};

void ra_scene_frequency_analyzer_on_enter(void* context) {
    RemoteAnalyzer* app = context;

    // The analyzer's worker drives the CC1101 itself, so the receiver has to be off it first
    ra_radio_stop(app->radio);

    app->fa_env.view_dispatcher = app->view_dispatcher;
    app->fa_env.view_id = RaViewIdPlugin;
    app->fa_env.park_view_id = RaViewIdSubmenu;
    app->fa_env.setting = ra_radio_get_setting(app->radio);
    app->fa_env.event_lock = RaCustomEventFaLock;
    app->fa_env.event_unlock = RaCustomEventFaUnlock;
    app->fa_env.event_picked = RaCustomEventFaPicked;
    app->fa_env.event_go_read = RaCustomEventFaGoRead;
    app->fa_env.trigger = app->settings.fa_trigger;
    app->fa_env.feedback = app->settings.fa_feedback;
    app->fa_env.frequency = 0;

    app->plugin_loader = ra_plugin_loader_load(
        RA_FA_PLUGIN_APP_ID,
        RA_FA_PLUGIN_API_VERSION,
        RA_PLUGIN_PATH("remote_analyzer_fa"),
        (const void**)&app->fa_plugin);
    if(app->fa_plugin == NULL) {
        // Back out from on_event, once the scene manager is done with this transition
        view_dispatcher_send_custom_event(app->view_dispatcher, RaCustomEventPluginMissing);
        return;
    }

    app->fa_plugin->on_enter(&app->fa_env);
}

bool ra_scene_frequency_analyzer_on_event(void* context, SceneManagerEvent event) {
    RemoteAnalyzer* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case RaCustomEventPluginMissing:
        scene_manager_previous_scene(app->scene_manager);
        return true;

    case RaCustomEventFaLock:
        notification_message(app->notifications, &sequence_set_green_255);
        switch(app->fa_env.feedback) {
        case RaFaFeedbackAll:
            notification_message(app->notifications, &sequence_success);
            break;
        case RaFaFeedbackVibro:
            notification_message(app->notifications, &sequence_single_vibro);
            break;
        default:
            break;
        }
        notification_message(app->notifications, &sequence_display_backlight_on);
        return true;

    case RaCustomEventFaUnlock:
        notification_message(app->notifications, &sequence_reset_rgb);
        return true;

    case RaCustomEventFaPicked:
        if(app->fa_env.frequency > 0) {
            /* A picked frequency is a decision, so hopping stops overriding it - otherwise
             * Read would ignore the one thing this screen was used to find. */
            app->settings.frequency = app->fa_env.frequency;
            app->settings.hopping = false;
            notification_message(app->notifications, &ra_sequence_picked);
        }
        return true;

    case RaCustomEventFaGoRead:
        if(app->fa_env.frequency > 0) {
            app->settings.frequency = app->fa_env.frequency;
            app->settings.hopping = false;
        }
        /* Drops this scene - and with it the analyzer's view, worker and mapped code - and
         * lands on Read with the picked frequency. */
        scene_manager_search_and_switch_to_another_scene(app->scene_manager, RaSceneRead);
        return true;

    default:
        return false;
    }
}

void ra_scene_frequency_analyzer_on_exit(void* context) {
    RemoteAnalyzer* app = context;

    if(app->fa_plugin != NULL) {
        // Joins the analyzer's worker, which runs plugin code, before the .fal is unmapped
        app->fa_plugin->on_exit(&app->fa_env);
        app->fa_plugin = NULL;

        app->settings.fa_trigger = app->fa_env.trigger;
        app->settings.fa_feedback = app->fa_env.feedback;
        ra_settings_save(&app->settings);
    }

    ra_plugin_loader_unload(app->plugin_loader);
    app->plugin_loader = NULL;

    notification_message(app->notifications, &sequence_reset_rgb);
}
