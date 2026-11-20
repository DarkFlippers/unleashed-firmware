#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>
#include <bt/bt_service/bt.h>
#include <extra_profiles/hid_profile.h>
#include <furi_ble/ams_client.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>

// Shared with the Remote app, so a phone paired there reconnects here without pairing again
#define KEYS_PATH EXT_PATH("apps_data/hid_ble/.bt_hid.keys")

typedef struct {
    FuriMutex* mutex;
    AmsInfo info;
    bool connected;
    uint32_t scroll;
    uint32_t info_tick;
    FuriString* title;
    FuriString* artist;
} NowPlaying;

typedef enum {
    EventInput,
    EventTick,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} Event;

static const char* latin1_ascii =
    "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYPsaaaaaaaceeeeiiiidnooooo/ouuuuypy";

// The built-in fonts are ASCII only, so fold UTF-8 to the nearest ASCII
static void to_ascii(FuriString* out, const char* in) {
    furi_string_reset(out);
    const uint8_t* p = (const uint8_t*)in;
    while(*p) {
        uint32_t cp;
        if(p[0] < 0x80) {
            cp = *p++;
        } else if((p[0] & 0xE0) == 0xC0 && p[1]) {
            cp = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
            p += 2;
        } else if((p[0] & 0xF0) == 0xE0 && p[1] && p[2]) {
            cp = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
            p += 3;
        } else {
            p++;
            while((*p & 0xC0) == 0x80)
                p++;
            cp = '?';
        }
        if(cp < 0x80) {
            furi_string_push_back(out, (char)cp);
        } else if(cp >= 0xC0 && cp <= 0xFF) {
            furi_string_push_back(out, latin1_ascii[cp - 0xC0]);
        } else if(cp == 0x2018 || cp == 0x2019) {
            furi_string_push_back(out, '\'');
        } else if(cp == 0x201C || cp == 0x201D) {
            furi_string_push_back(out, '"');
        } else if(cp == 0x2013 || cp == 0x2014) {
            furi_string_push_back(out, '-');
        } else {
            furi_string_push_back(out, '?');
        }
    }
}

// PlaybackInfo is "state,rate,elapsed"
static void parse_playback(const char* s, int* state, float* elapsed) {
    *state = atoi(s);
    *elapsed = 0;
    const char* c1 = strchr(s, ',');
    const char* c2 = c1 ? strchr(c1 + 1, ',') : NULL;
    if(c2) *elapsed = strtof(c2 + 1, NULL);
}

static void draw_callback(Canvas* canvas, void* ctx) {
    NowPlaying* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    canvas_clear(canvas);

    if(!app->connected) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 14, AlignCenter, AlignBottom, "Now Playing");
        canvas_set_font(canvas, FontSecondary);
        char hint[64];
        snprintf(
            hint,
            sizeof(hint),
            "Connect your iPhone:\nSettings > Bluetooth\n> Control %s",
            furi_hal_version_get_name_ptr());
        elements_multiline_text_aligned(canvas, 64, 26, AlignCenter, AlignTop, hint);
        furi_mutex_release(app->mutex);
        return;
    }

    AmsInfo* info = &app->info;
    if(info->state != AmsStateReady && info->state != AmsStateCommand) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 14, AlignCenter, AlignBottom, "Now Playing");
        canvas_set_font(canvas, FontSecondary);
        const char* msg = info->state == AmsStateUnsupported ? "Needs the Full BLE stack" :
                          info->state == AmsStateNotFound    ? "Phone has no media service" :
                          info->state == AmsStateError       ? "Media service error" :
                                                               "Connecting to media...";
        canvas_draw_str_aligned(canvas, 64, 36, AlignCenter, AlignCenter, msg);
        furi_mutex_release(app->mutex);
        return;
    }

    int state = 0;
    float elapsed = 0;
    parse_playback(info->playback, &state, &elapsed);
    if(state == 1) {
        elapsed += (furi_get_tick() - app->info_tick) / 1000.0f;
    }

    // Header: player and play state
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 9, info->player[0] ? info->player : "Media");
    char time_str[16];
    uint32_t secs = elapsed > 0 ? (uint32_t)elapsed : 0;
    if(secs > 359999) secs = 359999;
    snprintf(time_str, sizeof(time_str), "%lu:%02lu", secs / 60, secs % 60);
    canvas_draw_str_aligned(canvas, 126, 9, AlignRight, AlignBottom, time_str);
    if(info->queue_count > 0) {
        char queue_str[16];
        snprintf(
            queue_str,
            sizeof(queue_str),
            "%u/%u",
            (unsigned)(info->queue_index + 1),
            (unsigned)info->queue_count);
        canvas_draw_str_aligned(canvas, 82, 9, AlignRight, AlignBottom, queue_str);
    }
    canvas_draw_line(canvas, 0, 11, 127, 11);

    // Title and artist
    canvas_set_font(canvas, FontPrimary);
    if(furi_string_empty(app->title)) {
        canvas_draw_str(canvas, 2, 26, "Nothing playing");
    } else {
        elements_scrollable_text_line(canvas, 2, 26, 124, app->title, app->scroll, false);
    }
    canvas_set_font(canvas, FontSecondary);
    elements_scrollable_text_line(canvas, 2, 38, 124, app->artist, app->scroll, false);

    // Controls
    canvas_draw_line(canvas, 0, 50, 127, 50);
    canvas_draw_str(canvas, 2, 61, "<<");
    canvas_draw_str_aligned(
        canvas, 64, 61, AlignCenter, AlignBottom, state == 1 ? "OK: Pause" : "OK: Play");
    canvas_draw_str_aligned(canvas, 126, 61, AlignRight, AlignBottom, ">>");

    furi_mutex_release(app->mutex);
}

static void input_callback(InputEvent* input, void* ctx) {
    FuriMessageQueue* queue = ctx;
    Event event = {.type = EventInput, .input = *input};
    furi_message_queue_put(queue, &event, 0);
}

static void tick_callback(void* ctx) {
    FuriMessageQueue* queue = ctx;
    Event event = {.type = EventTick};
    furi_message_queue_put(queue, &event, 0);
}

static void bt_status_callback(BtStatus status, void* ctx) {
    NowPlaying* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    app->connected = (status == BtStatusConnected);
    furi_mutex_release(app->mutex);
}

static void refresh(NowPlaying* app) {
    AmsInfo fresh;
    ams_client_get(&fresh);
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    if(fresh.updates != app->info.updates || fresh.state != app->info.state) {
        bool track_changed = strcmp(fresh.title, app->info.title) != 0;
        app->info = fresh;
        app->info_tick = furi_get_tick();
        to_ascii(app->title, fresh.title);
        to_ascii(app->artist, fresh.artist);
        if(track_changed) app->scroll = 0;
    }
    app->scroll++;
    furi_mutex_release(app->mutex);
}

int32_t now_playing_app(void* p) {
    UNUSED(p);
    NowPlaying* app = malloc(sizeof(NowPlaying));
    memset(app, 0, sizeof(NowPlaying));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->title = furi_string_alloc();
    app->artist = furi_string_alloc();

    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(Event));
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, app);
    view_port_input_callback_set(view_port, input_callback, queue);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    Bt* bt = furi_record_open(RECORD_BT);
    bt_disconnect(bt);
    furi_delay_ms(200);
    bt_keys_storage_set_storage_path(bt, KEYS_PATH);
    FuriHalBleProfileBase* profile = bt_profile_start(bt, ble_profile_hid, NULL);
    furi_check(profile);
    furi_hal_bt_start_advertising();
    bt_set_status_changed_callback(bt, bt_status_callback, app);

    FuriTimer* timer = furi_timer_alloc(tick_callback, FuriTimerTypePeriodic, queue);
    furi_timer_start(timer, 300);

    Event event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(queue, &event, FuriWaitForever) != FuriStatusOk) continue;
        if(event.type == EventTick) {
            refresh(app);
        } else if(event.input.type == InputTypeShort || event.input.type == InputTypeRepeat) {
            switch(event.input.key) {
            case InputKeyOk:
                ams_client_command(AmsCommandTogglePlayPause);
                break;
            case InputKeyLeft:
                ams_client_command(AmsCommandPreviousTrack);
                break;
            case InputKeyRight:
                ams_client_command(AmsCommandNextTrack);
                break;
            case InputKeyUp:
                ams_client_command(AmsCommandVolumeUp);
                break;
            case InputKeyDown:
                ams_client_command(AmsCommandVolumeDown);
                break;
            case InputKeyBack:
                running = false;
                break;
            default:
                break;
            }
        }
        view_port_update(view_port);
    }

    furi_timer_stop(timer);
    furi_timer_free(timer);
    bt_set_status_changed_callback(bt, NULL, NULL);
    bt_disconnect(bt);
    furi_delay_ms(200);
    bt_keys_storage_set_default_path(bt);
    furi_check(bt_profile_restore_default(bt));
    furi_record_close(RECORD_BT);

    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(queue);
    furi_string_free(app->title);
    furi_string_free(app->artist);
    furi_mutex_free(app->mutex);
    free(app);
    return 0;
}
