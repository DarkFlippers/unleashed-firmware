#include "../remote_analyzer_i.h"
#include "ra_icons.h"

/* The plugin reports a kind per entry and indexes the icons below with it, so the two enums
 * have to stay in step. The .fal has its own copy of the list and no way to check this. */
static_assert((int)RaRawKindNone == (int)RaReadViewKindNone, "kind enums have diverged");
static_assert((int)RaRawKindUnknown == (int)RaReadViewKindUnknown, "kind enums have diverged");
static_assert((int)RaRawKindStatic == (int)RaReadViewKindStatic, "kind enums have diverged");
static_assert((int)RaRawKindDynamic == (int)RaReadViewKindDynamic, "kind enums have diverged");

/**
 * The noise a file decode makes, from the plugin's own thread of control. Audible feedback
 * follows the Notify setting; the LED does not, same as the live screen.
 */
static void ra_scene_decode_raw_notify(void* context, RaRawNotify what) {
    RemoteAnalyzer* app = context;

    switch(what) {
    case RaRawNotifyFound:
        if(app->settings.alert) notification_message(app->notifications, &sequence_success);
        notification_message(app->notifications, &sequence_display_backlight_on);
        break;

    case RaRawNotifyEmpty:
        if(app->settings.alert) notification_message(app->notifications, &sequence_error);
        /* Solid, not blinking, and held while the screen says so: there is nothing else on
         * it to look at. The error sequence above resets the LED on its way out, and this is
         * queued behind it, so what stays lit is this one. */
        notification_message(app->notifications, &sequence_set_only_red_255);
        notification_message(app->notifications, &sequence_display_backlight_on);
        break;

    case RaRawNotifyNotRaw:
        // The browser is coming straight back, so nothing is left blinking behind it
        if(app->settings.alert) notification_message(app->notifications, &sequence_error);
        break;
    }
}

/** Everything a file decode needs that the live screen does not comes from the .fal. */
void ra_scene_decode_raw_on_enter(void* context) {
    RemoteAnalyzer* app = context;

    /* The decoders are shared with the live screen, so the radio has to be off them before a
     * file starts feeding the same set. */
    ra_radio_stop(app->radio);
    ra_radio_reset_decoders(app->radio);

    app->raw_env.view_dispatcher = app->view_dispatcher;
    app->raw_env.view_id_progress = RaViewIdPlugin;
    app->raw_env.view_id_list = RaViewIdPluginList;
    app->raw_env.view_id_details = RaViewIdPluginDetails;
    app->raw_env.park_view_id = RaViewIdSubmenu;
    app->raw_env.dialogs = app->dialogs;
    app->raw_env.receiver = ra_radio_get_receiver(app->radio);
    /* The same formatting the live screen gets, so a reading out of a file reads exactly like
     * one off the air - support note, manufacturer and counter in the same places. */
    app->raw_env.format = remote_analyzer_format_signal;
    app->raw_env.icon_file = &I_sub1_10px;
    app->raw_env.icons[RaRawKindNone] = NULL;
    app->raw_env.icons[RaRawKindUnknown] = &I_Quest_7x8;
    app->raw_env.icons[RaRawKindStatic] = &I_Static_9x7;
    app->raw_env.icons[RaRawKindDynamic] = &I_Dynamic_9x7;
    app->raw_env.notify_context = app;
    app->raw_env.notify = ra_scene_decode_raw_notify;
    app->raw_env.event_cancelled = RaCustomEventRawCancelled;

    app->plugin_loader = ra_plugin_loader_load(
        RA_RAW_PLUGIN_APP_ID,
        RA_RAW_PLUGIN_API_VERSION,
        RA_PLUGIN_PATH("remote_analyzer_raw"),
        (const void**)&app->raw_plugin);
    if(app->raw_plugin == NULL) {
        // Back out from on_event, once the scene manager is done with this transition
        view_dispatcher_send_custom_event(app->view_dispatcher, RaCustomEventPluginMissing);
        return;
    }

    app->raw_plugin->on_enter(&app->raw_env);
    furi_timer_start(app->tick_timer, furi_ms_to_ticks(RA_SWEEP_TICK_MS));
}

bool ra_scene_decode_raw_on_event(void* context, SceneManagerEvent event) {
    RemoteAnalyzer* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case RaCustomEventPluginMissing:
    case RaCustomEventRawCancelled:
        scene_manager_previous_scene(app->scene_manager);
        return true;

    case RaCustomEventTick:
        /* The read runs here rather than in the timer callback: it drives the decoders, whose
         * callback collects the readings, and that belongs on the GUI thread. */
        if(app->raw_plugin != NULL) {
            // Nothing left to read, so the timer has nothing left to do either
            if(!app->raw_plugin->tick(&app->raw_env)) furi_timer_stop(app->tick_timer);
        }
        return true;

    default:
        return false;
    }
}

void ra_scene_decode_raw_on_exit(void* context) {
    RemoteAnalyzer* app = context;

    furi_timer_stop(app->tick_timer);
    notification_message(app->notifications, &sequence_reset_rgb);

    if(app->raw_plugin != NULL) {
        app->raw_plugin->on_exit(&app->raw_env);
        app->raw_plugin = NULL;
    }

    /* The decoders were reporting into the plugin, so they are pointed back at the app before
     * the code holding that callback is unmapped rather than after. */
    remote_analyzer_restore_rx_callback(app);
    ra_radio_reset_decoders(app->radio);

    ra_plugin_loader_unload(app->plugin_loader);
    app->plugin_loader = NULL;
}
