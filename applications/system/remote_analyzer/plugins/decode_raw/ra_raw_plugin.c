/**
 * @file ra_raw_plugin.c
 * @brief Entry point of the Decode RAW .fal: the picker, the read, the list and the details.
 */
#include "ra_raw.h"
#include "ra_raw_view.h"
#include "ra_raw_list_view.h"
#include "../../helpers/ra_raw_plugin.h"

#include <flipper_application/flipper_application.h>
#include <gui/modules/widget.h>
#include <toolbox/path.h>
#include <furi.h>

#define RA_RAW_DIR "/ext/subghz"
#define RA_RAW_EXT ".sub"

/* ra_raw_next covers a few hundred samples. One batch per tick would crawl through a long
 * capture, so several go through each time, which still leaves the screen its redraws. */
#define RA_RAW_BATCHES 8

/** The file name on the progress screen is centred FontSecondary and has the width to itself. */
#define RA_RAW_NAME_MAX 26

typedef struct {
    FuriString* text; /**< the reading as the decoder wrote it, CRLF separated */
    uint32_t count; /**< frames that produced exactly this reading */
    uint8_t kind; /**< RaRawKind, for the icon in the list */
} RaRawEntry;

/** One decode at a time, and each scene entry maps a fresh copy of the plugin. */
static struct {
    RaRaw* raw;
    FuriString* path;
    FuriString* name; /**< file name, cut to what the screen shows */
    FuriString* scratch; /**< the newest reading, before it is matched against the list */
    FuriString* out; /**< what the details screen is showing */

    /* A capture of a held down remote is one code over and over, so what fills this is
     * distinct readings, not frames. Past it the file is still read to the end and the counts
     * still rise, but no new entry is made. */
    RaRawEntry entries[RA_RAW_LIST_MAX];
    size_t count;
    bool full; /**< the list filled up, so the figure shown is a floor */
    uint32_t hz; /**< what the capture says it was recorded on, 0 when it does not say */

    RaRawView* progress;
    RaRawListView* list;
    Widget* details;
    uint32_t view_id_list; /**< for the details screen's own Back, which has no context */
} ra_raw_session;

/*
 * Runs on the GUI thread: the read below is what drives the decoders, so a frame is matched
 * against the list as it arrives and nothing has to cross a thread to get there.
 */
static void ra_raw_plugin_rx_callback(
    SubGhzReceiver* receiver,
    SubGhzProtocolDecoderBase* decoder_base,
    void* context) {
    RaRawEnv* env = context;
    uint8_t kind = RaRawKindNone;

    env->format(decoder_base, ra_raw_session.scratch, &kind);

    for(size_t i = 0; i < ra_raw_session.count; i++) {
        if(furi_string_equal(ra_raw_session.entries[i].text, ra_raw_session.scratch)) {
            ra_raw_session.entries[i].count++;
            subghz_receiver_reset(receiver);
            return;
        }
    }

    if(ra_raw_session.count < RA_RAW_LIST_MAX) {
        RaRawEntry* entry = &ra_raw_session.entries[ra_raw_session.count++];
        entry->text = furi_string_alloc_set(ra_raw_session.scratch);
        entry->count = 1;
        entry->kind = kind;
    } else {
        ra_raw_session.full = true;
    }

    // Drop every decoder's partial frame, so the next burst starts from a clean slate
    subghz_receiver_reset(receiver);
}

/** The list line: the reading's own headline, plus how many frames carried it. */
static void ra_raw_plugin_label(const RaRawEntry* entry, FuriString* out) {
    const char* text = furi_string_get_cstr(entry->text);
    size_t end = 0;
    while(text[end] && (text[end] != '\r') && (text[end] != '\n'))
        end++;

    furi_string_set_strn(out, text, end);
    if(entry->count > 1) furi_string_cat_printf(out, " x%lu", (unsigned long)entry->count);
}

/**
 * True where @p p is the space in front of a new field. A label is always a run of characters
 * closed by a colon, so this needs no measuring.
 */
static bool ra_raw_plugin_breaks_before(const char* p) {
    if(*p != ' ') return false;
    size_t n = 1;
    while(p[n] && (p[n] != ':') && (p[n] != ' ') && (p[n] != '\r') && (p[n] != '\n'))
        n++;
    return (n > 1) && (p[n] == ':');
}

/** The whole reading, as the details screen wants it. */
static void ra_raw_plugin_details(const RaRawEntry* entry, FuriString* out) {
    // One leading marker bolds the headline and nothing after it, which is the line's own end
    furi_string_set_str(out, "\e#");

    /* Two changes on the way through. The scroll element breaks on \n alone and would draw
     * the \r of a CRLF as a missing glyph, so the pair is cut to the byte it understands. And
     * it breaks per character, which would wrap a long hex value through the middle - so each
     * field gets a line of its own, which this screen has the height for anyway. */
    for(const char* p = furi_string_get_cstr(entry->text); *p; p++) {
        if(*p == '\r') continue;
        if(ra_raw_plugin_breaks_before(p)) {
            furi_string_push_back(out, '\n');
            continue;
        }
        furi_string_push_back(out, *p);
    }

    if(entry->count > 1) {
        furi_string_cat_printf(out, "\n\nSeen %lu times", (unsigned long)entry->count);
    }
}

static void ra_raw_plugin_list_callback(void* context, uint32_t index) {
    RaRawEnv* env = context;
    if(index >= ra_raw_session.count) return;

    ra_raw_plugin_details(&ra_raw_session.entries[index], ra_raw_session.out);
    widget_reset(ra_raw_session.details);
    widget_add_text_scroll_element(
        ra_raw_session.details, 0, 0, 128, 64, furi_string_get_cstr(ra_raw_session.out));
    view_dispatcher_switch_to_view(env->view_dispatcher, env->view_id_details);
}

/** Back on the details screen goes to the list, not out of the scene. */
static uint32_t ra_raw_plugin_back_to_list(void* context) {
    UNUSED(context);
    return ra_raw_session.view_id_list;
}

/**
 * A .sub is either a capture or a saved key, and only a capture carries samples. The browser
 * cannot tell them apart by name, so the pick is checked rather than failing silently on a
 * file that has nothing to decode.
 */
static void ra_raw_plugin_complain(RaRawEnv* env) {
    env->notify(env->notify_context, RaRawNotifyNotRaw);

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Not a RAW capture", 64, 8, AlignCenter, AlignTop);
    dialog_message_set_text(
        message,
        "Only a Read RAW capture\nholds samples to decode.\nThis file holds a key.",
        64,
        36,
        AlignCenter,
        AlignCenter);
    dialog_message_set_buttons(message, NULL, "OK", NULL);
    dialog_message_show(env->dialogs, message);
    dialog_message_free(message);
}

/** @return false when nothing was picked. @p frequency takes what the capture says. */
static bool ra_raw_plugin_pick(RaRawEnv* env, uint32_t* frequency) {
    DialogsFileBrowserOptions browser;
    dialog_file_browser_set_basic_options(&browser, RA_RAW_EXT, env->icon_file);
    browser.base_path = RA_RAW_DIR;

    furi_string_set_str(ra_raw_session.path, RA_RAW_DIR);

    // The browser owns the screen and the input while it is up
    while(dialog_file_browser_show(
        env->dialogs, ra_raw_session.path, ra_raw_session.path, &browser)) {
        if(ra_raw_file_is_raw(furi_string_get_cstr(ra_raw_session.path), frequency)) return true;
        // Back to the browser on the same directory, rather than out of the app's way
        ra_raw_plugin_complain(env);
    }
    return false;
}

/** The capture is read through: put what came out of it in the list. */
static void ra_raw_plugin_finish(RaRawEnv* env) {
    if(ra_raw_session.count == 0) {
        ra_raw_view_set_done(
            ra_raw_session.progress, "Nothing decoded", furi_string_get_cstr(ra_raw_session.name));
        env->notify(env->notify_context, RaRawNotifyEmpty);
        return;
    }

    FuriString* label = furi_string_alloc();
    for(size_t i = 0; i < ra_raw_session.count; i++) {
        ra_raw_plugin_label(&ra_raw_session.entries[i], label);
        ra_raw_list_view_add(
            ra_raw_session.list, furi_string_get_cstr(label), ra_raw_session.entries[i].kind);
    }

    // The capture's own frequency, since the list has nowhere else to say it
    furi_string_reset(label);
    if(ra_raw_session.hz) {
        furi_string_cat_printf(
            label,
            "%lu.%02lu  ",
            ra_raw_session.hz / 1000000,
            (ra_raw_session.hz % 1000000) / 10000);
    }
    furi_string_cat_printf(
        label,
        "%u%s signal%s",
        (unsigned int)ra_raw_session.count,
        ra_raw_session.full ? "+" : "",
        (ra_raw_session.count == 1) ? "" : "s");
    ra_raw_list_view_set_footer(ra_raw_session.list, furi_string_get_cstr(label));
    furi_string_free(label);

    view_dispatcher_switch_to_view(env->view_dispatcher, env->view_id_list);
    env->notify(env->notify_context, RaRawNotifyFound);
}

static void ra_raw_plugin_on_enter(RaRawEnv* env) {
    furi_assert(env);
    furi_assert(!ra_raw_session.raw);

    ra_raw_session.path = furi_string_alloc();
    ra_raw_session.name = furi_string_alloc();
    ra_raw_session.scratch = furi_string_alloc();
    ra_raw_session.out = furi_string_alloc();
    ra_raw_session.view_id_list = env->view_id_list;

    uint32_t hz = 0;
    if(!ra_raw_plugin_pick(env, &hz)) {
        // Back out from the scene's on_event, once the scene manager is done with us
        view_dispatcher_send_custom_event(env->view_dispatcher, env->event_cancelled);
        return;
    }

    ra_raw_session.hz = hz;
    path_extract_filename(ra_raw_session.path, ra_raw_session.name, true);
    furi_string_left(ra_raw_session.name, RA_RAW_NAME_MAX);

    ra_raw_session.progress = ra_raw_view_alloc();
    ra_raw_session.list = ra_raw_list_view_alloc();
    ra_raw_session.details = widget_alloc();

    ra_raw_list_view_set_icons(ra_raw_session.list, env->icons);
    ra_raw_list_view_set_callback(ra_raw_session.list, ra_raw_plugin_list_callback, env);
    view_set_previous_callback(
        widget_get_view(ra_raw_session.details), ra_raw_plugin_back_to_list);

    view_dispatcher_add_view(
        env->view_dispatcher,
        env->view_id_progress,
        ra_raw_view_get_view(ra_raw_session.progress));
    view_dispatcher_add_view(
        env->view_dispatcher, env->view_id_list, ra_raw_list_view_get_view(ra_raw_session.list));
    view_dispatcher_add_view(
        env->view_dispatcher, env->view_id_details, widget_get_view(ra_raw_session.details));

    ra_raw_session.raw = ra_raw_alloc();
    subghz_receiver_set_rx_callback(env->receiver, ra_raw_plugin_rx_callback, env);

    if(ra_raw_start(ra_raw_session.raw, furi_string_get_cstr(ra_raw_session.path), env->receiver)) {
        ra_raw_view_set_progress(
            ra_raw_session.progress, furi_string_get_cstr(ra_raw_session.name), 0);
    } else {
        ra_raw_view_set_done(
            ra_raw_session.progress, "Cannot open", furi_string_get_cstr(ra_raw_session.name));
        env->notify(env->notify_context, RaRawNotifyEmpty);
    }

    view_dispatcher_switch_to_view(env->view_dispatcher, env->view_id_progress);
}

static bool ra_raw_plugin_tick(RaRawEnv* env) {
    furi_assert(env);
    if(!ra_raw_session.raw || !ra_raw_is_running(ra_raw_session.raw)) return false;

    bool finished = false;
    for(uint8_t i = 0; (i < RA_RAW_BATCHES) && !finished; i++) {
        finished = !ra_raw_next(ra_raw_session.raw);
    }

    if(!finished) {
        ra_raw_view_set_progress(
            ra_raw_session.progress,
            furi_string_get_cstr(ra_raw_session.name),
            ra_raw_percent(ra_raw_session.raw));
        return true;
    }

    /* Stopped before the list is built, so the entries are final: the batch that runs the
     * file out can still decode on its way through. */
    ra_raw_stop(ra_raw_session.raw);
    ra_raw_plugin_finish(env);
    return false;
}

static void ra_raw_plugin_on_exit(RaRawEnv* env) {
    furi_assert(env);

    if(ra_raw_session.raw) {
        ra_raw_free(ra_raw_session.raw);
        ra_raw_session.raw = NULL;
    }

    if(ra_raw_session.progress) {
        /* Park on an app-owned view before dropping ours: removing the current view leaves
         * the dispatcher with none, which stops the event loop for good. */
        view_dispatcher_switch_to_view(env->view_dispatcher, env->park_view_id);
        view_dispatcher_remove_view(env->view_dispatcher, env->view_id_details);
        view_dispatcher_remove_view(env->view_dispatcher, env->view_id_list);
        view_dispatcher_remove_view(env->view_dispatcher, env->view_id_progress);

        widget_free(ra_raw_session.details);
        ra_raw_list_view_free(ra_raw_session.list);
        ra_raw_view_free(ra_raw_session.progress);
        ra_raw_session.details = NULL;
        ra_raw_session.list = NULL;
        ra_raw_session.progress = NULL;
    }

    for(size_t i = 0; i < ra_raw_session.count; i++) {
        furi_string_free(ra_raw_session.entries[i].text);
    }
    ra_raw_session.count = 0;
    ra_raw_session.full = false;
    ra_raw_session.hz = 0;

    furi_string_free(ra_raw_session.out);
    furi_string_free(ra_raw_session.scratch);
    furi_string_free(ra_raw_session.name);
    furi_string_free(ra_raw_session.path);
    ra_raw_session.out = NULL;
    ra_raw_session.scratch = NULL;
    ra_raw_session.name = NULL;
    ra_raw_session.path = NULL;
}

static const RaRawPlugin ra_raw_plugin = {
    .on_enter = ra_raw_plugin_on_enter,
    .tick = ra_raw_plugin_tick,
    .on_exit = ra_raw_plugin_on_exit,
};

static const FlipperAppPluginDescriptor ra_raw_plugin_descriptor = {
    .appid = RA_RAW_PLUGIN_APP_ID,
    .ep_api_version = RA_RAW_PLUGIN_API_VERSION,
    .entry_point = &ra_raw_plugin,
};

const FlipperAppPluginDescriptor* remote_analyzer_raw_ep(void) {
    return &ra_raw_plugin_descriptor;
}
