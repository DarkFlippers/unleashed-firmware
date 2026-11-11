#include "../remote_analyzer_i.h"

#include <furi_hal.h>

/* A decode is an instant: the green is held this long so it registers before the LED goes
 * back to reporting what is on the air. */
#define RA_SCENE_READ_GREEN_MS 1000

static void ra_scene_read_set_led(RemoteAnalyzer* app, uint8_t state) {
    if(app->led_state == state) return;
    app->led_state = state;

    switch(state) {
    case RaLedStateDecoded:
        notification_message(app->notifications, &sequence_blink_start_green);
        break;
    case RaLedStateSignal:
        notification_message(app->notifications, &sequence_blink_start_yellow);
        break;
    case RaLedStateReceiving:
        notification_message(app->notifications, &sequence_blink_start_magenta);
        break;
    default:
        notification_message(app->notifications, &sequence_blink_stop);
        break;
    }
}

/** Green while the decode is still fresh, yellow under a carrier, purple otherwise. */
static void ra_scene_read_update_led(RemoteAnalyzer* app, uint32_t now) {
    if(app->led_green_hold) {
        if((now - app->led_green_from) < furi_ms_to_ticks(RA_SCENE_READ_GREEN_MS)) {
            ra_scene_read_set_led(app, RaLedStateDecoded);
            return;
        }
        app->led_green_hold = false;
    }
    ra_scene_read_set_led(
        app, ra_sweep_is_signal(&app->sweep) ? RaLedStateSignal : RaLedStateReceiving);
}

static void ra_scene_read_view_callback(RaReadViewEvent event, void* context) {
    furi_assert(context);
    RemoteAnalyzer* app = context;
    view_dispatcher_send_custom_event(
        app->view_dispatcher,
        event == RaReadViewEventConfig ? RaCustomEventReadConfig : RaCustomEventReadReset);
}

/** How many frequencies hopping has to work with; under two there is nothing to hop between. */
static size_t ra_scene_read_hop_count(RemoteAnalyzer* app) {
    if(!app->settings.hopping) return 0;
    return subghz_setting_get_hopper_frequency_count(ra_radio_get_setting(app->radio));
}

/** The frequency to be on: the hopper's current one, or the chosen one when it is not hopping. */
static uint32_t ra_scene_read_frequency(RemoteAnalyzer* app) {
    const size_t count = ra_scene_read_hop_count(app);
    if(count < 2) return app->settings.frequency;

    SubGhzSetting* setting = ra_radio_get_setting(app->radio);
    size_t index = ra_sweep_get_hop_index(&app->sweep);
    if(index >= count) index = 0;
    return subghz_setting_get_hopper_frequency(setting, index);
}

/**
 * A modulation still being tried gets a marker, so the header is readable as what is being
 * listened to now rather than as the answer.
 */
static void ra_scene_read_update_header(RemoteAnalyzer* app) {
    FuriString* frequency = furi_string_alloc();
    FuriString* modulation = furi_string_alloc();
    ra_radio_get_frequency_and_modulation(app->radio, frequency, modulation);

    if(ra_sweep_is_sweeping(&app->sweep)) {
        // Preset names are five characters, so this cannot be what cuts the header short
        char marked[16] = {0};
        snprintf(marked, sizeof(marked), "~%s", furi_string_get_cstr(modulation));
        furi_string_set_str(modulation, marked);
    }
    ra_read_view_set_radio(
        app->read_view, furi_string_get_cstr(frequency), furi_string_get_cstr(modulation));

    furi_string_free(modulation);
    furi_string_free(frequency);
}

/** Take the receiver to the frequency and preset the sweep is asking for. */
static void ra_scene_read_retune(RemoteAnalyzer* app) {
    const size_t preset = ra_sweep_get_preset(&app->sweep);
    const uint32_t frequency = ra_scene_read_frequency(app);

    /* RX down first: the decoders are reset from here, the GUI thread, while the worker
     * thread is the one feeding them. */
    ra_radio_stop(app->radio);
    ra_radio_reset_decoders(app->radio);
    ra_radio_set_preset(app->radio, frequency, preset);
    ra_radio_rx_start(app->radio);

    app->tuned_preset_index = preset;
    ra_scene_read_update_header(app);
}

/** Mirror the sweep's state onto the screen. Only the cycle's own verdict has anything to say. */
static void ra_scene_read_update_sweep(RemoteAnalyzer* app) {
    const char* status = ra_sweep_get_status(&app->sweep);
    ra_read_view_set_status(app->read_view, status, status ? "Nothing decoded it" : NULL);
}

void ra_scene_read_on_enter(void* context) {
    RemoteAnalyzer* app = context;

    /* Coming back with a different frequency, or back from a file decode, makes whatever is
     * on screen belong to something we are no longer listening to. */
    if(app->tuned_frequency != app->settings.frequency ||
       app->tuned_preset_index != app->settings.preset_index) {
        ra_read_view_clear(app->read_view);
        remote_analyzer_forget_last_signal(app);
    }
    app->tuned_frequency = app->settings.frequency;
    app->tuned_preset_index = app->settings.preset_index;

    const RaSweepConfig config = {
        .sweep = app->settings.sweep,
        .hop = app->settings.hopping,
        .trigger = app->settings.rssi_trigger,
        .preset_index = app->settings.preset_index,
        .preset_count = subghz_setting_get_preset_count(ra_radio_get_setting(app->radio)),
        .hop_count = ra_scene_read_hop_count(app),
        .ticks_per_second = furi_kernel_get_tick_frequency(),
    };
    ra_sweep_init(&app->sweep, &config);

    ra_read_view_set_status(app->read_view, NULL, NULL);
    ra_read_view_set_callback(app->read_view, ra_scene_read_view_callback, app);

    ra_radio_set_speaker(app->radio, app->settings.sound);
    ra_scene_read_retune(app);

    furi_timer_start(app->tick_timer, furi_ms_to_ticks(RA_SWEEP_TICK_MS));

    app->led_green_hold = false;
    app->led_state = RaLedStateNone;
    ra_scene_read_update_led(app, furi_get_tick());

    view_dispatcher_switch_to_view(app->view_dispatcher, RaViewIdRead);
}

bool ra_scene_read_on_event(void* context, SceneManagerEvent event) {
    RemoteAnalyzer* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case RaCustomEventSignal: {
        furi_mutex_acquire(app->signal_mutex, FuriWaitForever);
        ra_read_view_set_signal(
            app->read_view, furi_string_get_cstr(app->signal_text), app->signal_kind);
        const bool fresh = app->signal_fresh;
        app->signal_fresh = false;
        furi_mutex_release(app->signal_mutex);

        const uint32_t now = furi_get_tick();
        /* A decode ends the cycle where it stands: this frequency and this modulation are
         * the ones that work, so the header stops moving and says so. */
        ra_sweep_decoded(&app->sweep, now);
        ra_scene_read_update_sweep(app);
        ra_scene_read_update_header(app);

        app->led_green_from = now;
        app->led_green_hold = true;
        ra_scene_read_update_led(app, now);

        if(fresh && app->settings.alert) {
            notification_message(app->notifications, &ra_sequence_new_signal);
        }
        notification_message(app->notifications, &sequence_display_backlight_on);
        return true;
    }

    case RaCustomEventTick: {
        const uint32_t now = furi_get_tick();
        const float rssi = furi_hal_subghz_get_rssi();
        ra_read_view_set_rssi(app->read_view, rssi);

        // Read before the retune, so the next reading is a whole tick after the hop
        const RaSweepAction action = ra_sweep_tick(&app->sweep, rssi, now);
        if(action == RaSweepActionRetune) ra_scene_read_retune(app);

        ra_scene_read_update_sweep(app);
        ra_scene_read_update_header(app);
        ra_scene_read_update_led(app, now);
        return true;
    }

    case RaCustomEventReadConfig:
        scene_manager_next_scene(app->scene_manager, RaSceneReadConfig);
        return true;

    case RaCustomEventReadReset:
        ra_sweep_reset(&app->sweep, furi_get_tick());
        ra_scene_read_retune(app);
        ra_read_view_clear(app->read_view);
        remote_analyzer_forget_last_signal(app);
        ra_scene_read_update_sweep(app);
        app->led_green_hold = false;
        ra_scene_read_update_led(app, furi_get_tick());
        return true;

    default:
        return false;
    }
}

void ra_scene_read_on_exit(void* context) {
    RemoteAnalyzer* app = context;
    furi_timer_stop(app->tick_timer);
    ra_radio_stop(app->radio);
    ra_scene_read_set_led(app, RaLedStateNone);

    /* Leaving mid-cycle would otherwise come back on whatever the hop and the sweep had
     * reached, which is not what the config says is selected. */
    ra_sweep_reset(&app->sweep, furi_get_tick());
    ra_radio_set_preset(app->radio, app->settings.frequency, app->settings.preset_index);
    app->tuned_preset_index = app->settings.preset_index;
}
