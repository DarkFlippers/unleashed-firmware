#include "remote_analyzer_i.h"

#include "protocols/ra_protocol_items.h"

#include <lib/subghz/protocols/base.h>
#include <lib/subghz/protocols/nero_radio.h>
#include <furi_hal_power.h>
#include <string.h>

#define TAG "RemoteAnalyzer"

/* Vibro and a short chirp. Deliberately not sequence_success, which also drives the LED and
 * the backlight and so would fight the state blink. */
const NotificationSequence ra_sequence_new_signal = {
    &message_vibro_on,
    &message_note_c5,
    &message_delay_50,
    &message_vibro_off,
    &message_note_e5,
    &message_delay_50,
    &message_sound_off,
    NULL,
};

/** Asks the decoder rather than reading the protocol's declared type, so a protocol with more
 * than one frame format is reported per signal. */
static uint8_t remote_analyzer_signal_kind(SubGhzProtocolDecoderBase* decoder_base) {
    switch(subghz_protocol_decoder_base_get_type(decoder_base)) {
    case SubGhzProtocolTypeStatic:
        return RaReadViewKindStatic;
    case SubGhzProtocolTypeDynamic:
        return RaReadViewKindDynamic;
    default:
        return RaReadViewKindUnknown;
    }
}

/** Where the decoded protocol stands in the SubGhz app, as the last line of the reading. */
static const char* remote_analyzer_support_note(SubGhzProtocolDecoderBase* decoder_base) {
    const SubGhzProtocol* protocol = decoder_base->protocol;
    if(protocol == NULL || protocol->name == NULL || ra_protocol_is_own(protocol)) {
        return "Not supported in SubGHz";
    }

    /* Nero Radio's rolling formats decode and replay, but their counter is not tracked, so
     * calling them supported outright would overstate it. */
    if((strcmp(protocol->name, SUBGHZ_PROTOCOL_NERO_RADIO_NAME) == 0) &&
       (subghz_protocol_decoder_base_get_type(decoder_base) == SubGhzProtocolTypeDynamic)) {
        return "SubGHz: decode/replay\r\nonly, not full rolling";
    }

    return "Supported in SubGHz Read";
}

/**
 * Put @p note directly under the headline, ahead of the key and the other fields, so it is on
 * screen without having to scroll the reading.
 */
static void remote_analyzer_insert_note(FuriString* text, const char* note) {
    const size_t size = furi_string_size(text);
    size_t end = 0;
    while((end < size) && (furi_string_get_char(text, end) != '\r') &&
          (furi_string_get_char(text, end) != '\n')) {
        end++;
    }

    // In this order, so the note lands after the break rather than before it
    furi_string_replace_at(text, end, 0, note);
    if(end) furi_string_replace_at(text, end, 0, "\r\n");
}

/** Most readings are 3 to 6 lines; past this the text is left alone rather than reshuffled. */
#define RA_SIGNAL_MAX_LINES 12

/**
 * Move the line starting with @p key to index @p to. KeeLoq and friends print the manufacturer
 * last, which is where a four line screen is least likely to show it.
 *
 * @return false when there is no such line below @p to, or the reading is too long to reorder
 */
static bool remote_analyzer_hoist_line(FuriString* text, const char* key, size_t to) {
    const char* line[RA_SIGNAL_MAX_LINES];
    size_t len[RA_SIGNAL_MAX_LINES];
    size_t count = 0;

    const char* p = furi_string_get_cstr(text);
    while(*p && (count < RA_SIGNAL_MAX_LINES)) {
        const char* end = p;
        while(*end && (*end != '\r') && (*end != '\n'))
            end++;
        line[count] = p;
        len[count] = (size_t)(end - p);
        count++;
        while((*end == '\r') || (*end == '\n'))
            end++;
        p = end;
    }
    if(*p) return false;
    if((to == 0) || (to >= count)) return false;

    // From to + 1: anything at or above to is already in place
    const size_t key_len = strlen(key);
    size_t at = 0;
    for(size_t i = to + 1; i < count; i++) {
        if((len[i] >= key_len) && (strncmp(line[i], key, key_len) == 0)) {
            at = i;
            break;
        }
    }
    if(at == 0) return false;

    FuriString* out = furi_string_alloc();
    size_t next = 0; // next line to copy, skipping the one being moved
    for(size_t slot = 0; slot < count; slot++) {
        size_t take;
        if(slot == to) {
            take = at;
        } else {
            while(next == at)
                next++;
            take = next++;
        }
        if(slot) furi_string_cat_str(out, "\r\n");
        furi_string_cat_printf(out, "%.*s", (int)len[take], line[take]);
    }
    furi_string_set(text, out);
    furi_string_free(out);
    return true;
}

/** Value following @p key, up to the next space. @p key has to start a word. */
static bool remote_analyzer_field(const FuriString* text, const char* key, FuriString* out) {
    const char* s = furi_string_get_cstr(text);
    const size_t key_len = strlen(key);

    for(const char* p = strstr(s, key); p; p = strstr(p + 1, key)) {
        if((p != s) && (p[-1] != ' ') && (p[-1] != '\r') && (p[-1] != '\n')) continue;
        p += key_len;
        while(*p == ' ')
            p++;
        size_t n = 0;
        while(p[n] && (p[n] != ' ') && (p[n] != '\r') && (p[n] != '\n'))
            n++;
        if(n == 0) return false;
        furi_string_set_strn(out, p, n);
        return true;
    }
    return false;
}

/**
 * KeeLoq carries the counter inside its Fix line and the button inside its Hop line, both of
 * which a four line screen pushes out of sight. This gives the pair a line of their own under
 * the manufacturer, leaving the lines they came from as they were.
 */
static void remote_analyzer_promote_counter(FuriString* text) {
    if(remote_analyzer_hoist_line(text, "Cnt:", 2)) return;

    FuriString* cnt = furi_string_alloc();
    FuriString* btn = furi_string_alloc();
    if(remote_analyzer_field(text, "Cnt:", cnt) && remote_analyzer_field(text, "Btn:", btn)) {
        // Added at the end and then lifted, so the one mover covers both shapes
        furi_string_cat_printf(
            text, "\r\nCnt:%s Btn:%s", furi_string_get_cstr(cnt), furi_string_get_cstr(btn));
        remote_analyzer_hoist_line(text, "Cnt:", 2);
    }
    furi_string_free(btn);
    furi_string_free(cnt);
}

void remote_analyzer_format_signal(
    SubGhzProtocolDecoderBase* decoder_base,
    FuriString* text,
    uint8_t* kind) {
    furi_string_reset(text);
    subghz_protocol_decoder_base_get_string(decoder_base, text);
    remote_analyzer_insert_note(text, remote_analyzer_support_note(decoder_base));
    /* After the note, so the manufacturer ends up the line right under the headline. Only the
     * KeeLoq family prints MF, and it is the same one that buries the counter and button. */
    if(remote_analyzer_hoist_line(text, "MF:", 1)) {
        remote_analyzer_promote_counter(text);
    }
    if(kind) *kind = remote_analyzer_signal_kind(decoder_base);
}

/** Runs on the capture worker's thread. Keep it to building the text and waking the GUI. */
static void remote_analyzer_rx_callback(
    SubGhzReceiver* receiver,
    SubGhzProtocolDecoderBase* decoder_base,
    void* context) {
    furi_assert(context);
    RemoteAnalyzer* app = context;

    furi_mutex_acquire(app->signal_mutex, FuriWaitForever);
    remote_analyzer_format_signal(decoder_base, app->signal_text, &app->signal_kind);
    /* The text carries the key and any counter, so an identical one is the same press
     * repeating rather than a code worth buzzing about again. Sticky until the screen clears
     * it, so one new code in a run of repeats is still announced. */
    if(!furi_string_equal(app->signal_text, app->signal_prev)) app->signal_fresh = true;
    furi_string_set(app->signal_prev, app->signal_text);
    furi_mutex_release(app->signal_mutex);

    // Drop every decoder's partial frame, so the next press starts from a clean slate
    subghz_receiver_reset(receiver);

    view_dispatcher_send_custom_event(app->view_dispatcher, RaCustomEventSignal);
}

void remote_analyzer_restore_rx_callback(RemoteAnalyzer* app) {
    furi_assert(app);
    ra_radio_set_rx_callback(app->radio, remote_analyzer_rx_callback, app);
}

void remote_analyzer_forget_last_signal(RemoteAnalyzer* app) {
    furi_assert(app);
    furi_mutex_acquire(app->signal_mutex, FuriWaitForever);
    furi_string_reset(app->signal_prev);
    app->signal_fresh = false;
    furi_mutex_release(app->signal_mutex);
}

static bool remote_analyzer_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    RemoteAnalyzer* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool remote_analyzer_back_event_callback(void* context) {
    furi_assert(context);
    RemoteAnalyzer* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

/** Drives the Read screen's RSSI bar and sweep, and the file pump behind Decode RAW. */
static void remote_analyzer_tick_timer_callback(void* context) {
    furi_assert(context);
    RemoteAnalyzer* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, RaCustomEventTick);
}

static RemoteAnalyzer* remote_analyzer_alloc(void) {
    RemoteAnalyzer* app = malloc(sizeof(RemoteAnalyzer));

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&ra_scene_handlers, app);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, remote_analyzer_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, remote_analyzer_back_event_callback);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, RaViewIdSubmenu, submenu_get_view(app->submenu));

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        RaViewIdVarItemList,
        variable_item_list_get_view(app->var_item_list));

    app->read_view = ra_read_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, RaViewIdRead, ra_read_view_get_view(app->read_view));

    app->signal_text = furi_string_alloc();
    app->signal_prev = furi_string_alloc();
    app->signal_fresh = false;
    app->signal_kind = RaReadViewKindNone;
    app->led_state = RaLedStateNone;
    app->led_green_from = 0;
    app->led_green_hold = false;
    app->tick_timer =
        furi_timer_alloc(remote_analyzer_tick_timer_callback, FuriTimerTypePeriodic, app);
    app->signal_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    app->radio = ra_radio_alloc();
    ra_radio_set_rx_callback(app->radio, remote_analyzer_rx_callback, app);

    ra_settings_load(
        &app->settings, subghz_setting_get_preset_count(ra_radio_get_setting(app->radio)));

    /* Nothing is tuned yet, and no frequency is 0, so the first entry into Read counts as a
     * retune and starts from an empty screen. */
    app->tuned_frequency = 0;
    app->tuned_preset_index = UINT32_MAX;

    app->plugin_loader = NULL;
    app->fa_plugin = NULL;
    app->raw_plugin = NULL;
    app->about_plugin = NULL;

    return app;
}

static void remote_analyzer_free(RemoteAnalyzer* app) {
    furi_assert(app);

    // A plugin scene unloads on its way out, but a crash-free exit path should not depend on
    // that having happened
    ra_plugin_loader_unload(app->plugin_loader);
    app->plugin_loader = NULL;
    app->fa_plugin = NULL;
    app->raw_plugin = NULL;
    app->about_plugin = NULL;

    ra_radio_free(app->radio);

    view_dispatcher_remove_view(app->view_dispatcher, RaViewIdRead);
    ra_read_view_free(app->read_view);
    view_dispatcher_remove_view(app->view_dispatcher, RaViewIdVarItemList);
    variable_item_list_free(app->var_item_list);
    view_dispatcher_remove_view(app->view_dispatcher, RaViewIdSubmenu);
    submenu_free(app->submenu);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_timer_free(app->tick_timer);
    furi_mutex_free(app->signal_mutex);
    furi_string_free(app->signal_text);
    furi_string_free(app->signal_prev);

    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t remote_analyzer_app(void* p) {
    UNUSED(p);

    RemoteAnalyzer* app = remote_analyzer_alloc();

    scene_manager_next_scene(app->scene_manager, RaSceneStart);

    // Charging noise sits right on top of what the receiver is trying to hear
    furi_hal_power_suppress_charge_enter();
    view_dispatcher_run(app->view_dispatcher);
    furi_hal_power_suppress_charge_exit();

    ra_settings_save(&app->settings);
    remote_analyzer_free(app);

    return 0;
}
