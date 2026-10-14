/*
 * Seed Capturer — collect the Fix + Hop pairs an offline seed recovery needs.
 *
 * FAAC SLH, Genius and Erreka remotes all encrypt a counter with a manufacturer key
 * derived from a per installation seed. Recovering that seed is a search, and a search
 * needs more than one frame from the same remote: a single Fix + Hop pair fits far too
 * many seeds. This app listens, keeps every distinct Hop that arrives under one Fix, and
 * writes them out as a plain text file that the solver reads later. It never transmits
 * and it never tries to solve anything itself.
 */

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>
#include <datetime/datetime.h>

#include <lib/subghz/receiver.h>
#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/environment.h>
#include <lib/subghz/blocks/math.h>
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/protocols/faac_slh.h>
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/devices/devices.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>
#include <flipper_format/flipper_format.h>

#define TAG           "SeedCapturer"
#define SEED_FOLDER   "/ext/apps_data/subghz_seed_captures"
#define SEED_PARENT   "/ext/apps_data"
#define SEED_EXT      ".txt"
#define SEED_PRESET   "FuriHalSubGhzPresetOok650Async"
#define SEED_MIN_HOP  2
#define SEED_MAX_HOP  10
#define SEED_PATH_LEN 128

typedef enum {
    SeedMfrFaacSlh,
    SeedMfrGenius,
    SeedMfrErreka,
    SeedMfrCount,
} SeedMfr;

typedef struct {
    const char* label; // menu entry and capture screen title
    const char* prefix; // file name prefix
    const char* protocol; // decoder that carries this remote
    /* KeeLoq goes out LSB first, so the raw key has to be reversed before Fix and Hop
     * line up with the halves the rest of the world quotes. Faac SLH does not. */
    bool reverse_key;
} SeedMfrInfo;

static const SeedMfrInfo seed_mfr[SeedMfrCount] = {
    [SeedMfrFaacSlh] = {"FAAC SLH", "FAAC", SUBGHZ_PROTOCOL_FAAC_SLH_NAME, false},
    [SeedMfrGenius] = {"Genius", "Genius", SUBGHZ_PROTOCOL_FAAC_SLH_NAME, false},
    [SeedMfrErreka] = {"Erreka", "Erreka", SUBGHZ_PROTOCOL_KEELOQ_NAME, true},
};

typedef struct {
    uint32_t hz;
    const char* label;
} SeedFreqInfo;

static const SeedFreqInfo seed_freq[] = {
    {433920000, "433.92 MHz  AM650"},
    {868350000, "868.35 MHz  AM650"},
};

typedef enum {
    SeedSceneMenu,
    SeedSceneMfr,
    SeedSceneFreq,
    SeedSceneCapture,
    SeedSceneWipeConfirm,
    SeedSceneResult,
} SeedScene;

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* queue;
    ViewPort* view_port;
    Gui* gui;
    Storage* storage;
    NotificationApp* notifications;

    SeedScene scene;
    uint8_t menu_index;
    uint8_t mfr_index;
    uint8_t freq_index;
    FuriString* status;

    SubGhzEnvironment* environment;
    SubGhzReceiver* receiver;
    SubGhzWorker* worker;
    const SubGhzDevice* device;
    bool rx_active;

    // capture, all of it under one Fix
    bool has_fix;
    uint32_t fix;
    uint32_t hops[SEED_MAX_HOP];
    uint8_t hop_count;
    uint32_t packets;
    DateTime first_seen;
} SeedCapturer;

/* ------------------------------------------------------------------ drawing */

static const char* const seed_menu[] = {"New capture", "Delete all captures", "Exit"};

static void seed_draw_menu(Canvas* canvas, const char* const* items, uint8_t count, uint8_t sel) {
    for(uint8_t i = 0; i < count; i++) {
        if(i == sel) {
            canvas_draw_box(canvas, 0, 17 + i * 12, 128, 11);
            canvas_set_color(canvas, ColorWhite);
        }
        canvas_draw_str(canvas, 4, 26 + i * 12, items[i]);
        canvas_set_color(canvas, ColorBlack);
    }
}

static void seed_draw_callback(Canvas* canvas, void* ctx) {
    SeedCapturer* app = ctx;
    char buf[40];
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    switch(app->scene) {
    case SeedSceneMfr:
        canvas_draw_str(canvas, 2, 10, "Remote type");
        break;
    case SeedSceneFreq:
        canvas_draw_str(canvas, 2, 10, "Frequency");
        break;
    case SeedSceneCapture:
        snprintf(
            buf,
            sizeof(buf),
            "%s  %s",
            seed_mfr[app->mfr_index].label,
            app->freq_index ? "868.35" : "433.92");
        canvas_draw_str(canvas, 2, 10, buf);
        break;
    default:
        canvas_draw_str(canvas, 2, 10, "Seed Capturer");
        break;
    }
    canvas_draw_line(canvas, 0, 13, 128, 13);
    canvas_set_font(canvas, FontSecondary);

    switch(app->scene) {
    case SeedSceneMenu:
        seed_draw_menu(canvas, seed_menu, COUNT_OF(seed_menu), app->menu_index);
        break;

    case SeedSceneMfr: {
        const char* items[SeedMfrCount];
        for(uint8_t i = 0; i < SeedMfrCount; i++) {
            items[i] = seed_mfr[i].label;
        }
        seed_draw_menu(canvas, items, SeedMfrCount, app->mfr_index);
        break;
    }

    case SeedSceneFreq: {
        const char* items[COUNT_OF(seed_freq)];
        for(uint8_t i = 0; i < COUNT_OF(seed_freq); i++) {
            items[i] = seed_freq[i].label;
        }
        seed_draw_menu(canvas, items, COUNT_OF(seed_freq), app->freq_index);
        break;
    }

    case SeedSceneCapture:
        if(app->has_fix) {
            snprintf(buf, sizeof(buf), "Fix  %08lX", app->fix);
        } else {
            snprintf(buf, sizeof(buf), "Fix  --------");
        }
        canvas_draw_str(canvas, 2, 23, buf);
        snprintf(
            buf, sizeof(buf), "Hops %u/%u    %lupkt", app->hop_count, SEED_MAX_HOP, app->packets);
        canvas_draw_str(canvas, 2, 33, buf);

        elements_frame(canvas, 0, 37, 128, 14);
        canvas_draw_str(canvas, 4, 47, furi_string_get_cstr(app->status));

        if(app->hop_count >= SEED_MIN_HOP) elements_button_center(canvas, "Save");
        elements_button_left(canvas, "Reset");
        break;

    case SeedSceneWipeConfirm:
        elements_multiline_text_aligned(
            canvas,
            64,
            28,
            AlignCenter,
            AlignCenter,
            "Delete every capture file\nin subghz_seed_captures?");
        elements_button_center(canvas, "Delete");
        elements_button_left(canvas, "Cancel");
        break;

    default:
        /* a saved file name is wider than the screen, so let the text box wrap it */
        elements_text_box(
            canvas, 2, 16, 124, 34, AlignCenter, AlignTop, furi_string_get_cstr(app->status), false);
        elements_button_left(canvas, "Back");
        break;
    }

    furi_mutex_release(app->mutex);
}

static void seed_input_callback(InputEvent* event, void* ctx) {
    FuriMessageQueue* queue = ctx;
    furi_message_queue_put(queue, event, FuriWaitForever);
}

static void seed_set_status(SeedCapturer* app, const char* text) {
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    furi_string_set(app->status, text);
    furi_mutex_release(app->mutex);
}

/* ------------------------------------------------------------------ capture */

/* Pull the raw 64 bit key out through the public serializer rather than reaching into
 * decoder internals, the way the sub-ghz app itself reads a decoded frame. */
static bool seed_read_key(SubGhzProtocolDecoderBase* base, uint32_t frequency, uint64_t* key) {
    FlipperFormat* fff = flipper_format_string_alloc();
    SubGhzRadioPreset preset = {
        .name = furi_string_alloc_set(SEED_PRESET),
        .frequency = frequency,
        .data = NULL,
        .data_size = 0,
    };

    uint8_t key_data[8] = {0};
    uint32_t bits = 0;
    bool ok = false;
    if((subghz_protocol_decoder_base_serialize(base, fff, &preset) == SubGhzProtocolStatusOk) &&
       flipper_format_rewind(fff) && flipper_format_read_uint32(fff, "Bit", &bits, 1) &&
       (bits == 64) && flipper_format_rewind(fff) &&
       flipper_format_read_hex(fff, "Key", key_data, sizeof(key_data))) {
        uint64_t value = 0;
        for(size_t i = 0; i < sizeof(key_data); i++) {
            value = (value << 8) | key_data[i];
        }
        *key = value;
        ok = true;
    }

    furi_string_free(preset.name);
    flipper_format_free(fff);
    return ok;
}

/* Runs on the SubGhzWorker thread. mfr_index and freq_index only ever change from the
 * menus, with the radio stopped, so reading them here needs no lock. */
static void
    seed_rx_callback(SubGhzReceiver* receiver, SubGhzProtocolDecoderBase* base, void* ctx) {
    UNUSED(receiver);
    SeedCapturer* app = ctx;
    const SeedMfrInfo* mfr = &seed_mfr[app->mfr_index];

    if(strcmp(base->protocol->name, mfr->protocol) != 0) return;

    uint64_t key = 0;
    if(!seed_read_key(base, seed_freq[app->freq_index].hz, &key)) return;
    if(mfr->reverse_key) key = subghz_protocol_blocks_reverse_key(key, 64);

    const uint32_t fix = (uint32_t)(key >> 32);
    const uint32_t hop = (uint32_t)key;
    /* Faac SLH also puts out frames with an empty Fix while a remote is being paired.
     * They carry no serial, so they cannot be attributed to a remote. */
    if((fix == 0) || (hop == 0)) return;

    furi_mutex_acquire(app->mutex, FuriWaitForever);
    app->packets++;

    if(!app->has_fix) {
        app->has_fix = true;
        app->fix = fix;
        furi_hal_rtc_get_datetime(&app->first_seen);
    }

    if(fix != app->fix) {
        /* Another remote in range. Locking onto the first Fix seen is what keeps one
         * file to one remote; Reset re-arms if it locked onto the wrong one. */
        furi_string_printf(app->status, "Other remote %08lX", fix);
    } else if(app->hop_count >= SEED_MAX_HOP) {
        furi_string_printf(app->status, "Have %u, enough", SEED_MAX_HOP);
    } else {
        bool known = false;
        for(uint8_t i = 0; i < app->hop_count; i++) {
            if(app->hops[i] == hop) known = true;
        }
        if(known) {
            furi_string_set(app->status, "Same hop, ignored");
        } else {
            app->hops[app->hop_count++] = hop;
            furi_string_printf(app->status, "Hop %u: %08lX", app->hop_count, hop);
            notification_message(app->notifications, &sequence_blink_green_10);
        }
    }
    furi_mutex_release(app->mutex);
}

static bool seed_rx_start(SeedCapturer* app) {
    app->environment = subghz_environment_alloc();
    subghz_environment_set_protocol_registry(app->environment, (void*)&subghz_protocol_registry);

    app->receiver = subghz_receiver_alloc_init(app->environment);
    subghz_receiver_set_filter(app->receiver, SubGhzProtocolFlag_Decodable);
    subghz_receiver_set_rx_callback(app->receiver, seed_rx_callback, app);

    subghz_devices_init();
    app->device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);
    if(!app->device) return false;

    const uint32_t frequency = seed_freq[app->freq_index].hz;
    if(!subghz_devices_is_frequency_valid(app->device, frequency)) return false;

    subghz_devices_begin(app->device);
    subghz_devices_reset(app->device);
    subghz_devices_load_preset(app->device, FuriHalSubGhzPresetOok650Async, NULL);
    subghz_devices_set_frequency(app->device, frequency);

    app->worker = subghz_worker_alloc();
    subghz_worker_set_overrun_callback(
        app->worker, (SubGhzWorkerOverrunCallback)subghz_receiver_reset);
    subghz_worker_set_pair_callback(app->worker, (SubGhzWorkerPairCallback)subghz_receiver_decode);
    subghz_worker_set_context(app->worker, app->receiver);

    subghz_devices_start_async_rx(app->device, subghz_worker_rx_callback, app->worker);
    subghz_worker_start(app->worker);
    subghz_devices_set_rx(app->device);
    app->rx_active = true;
    return true;
}

static void seed_rx_stop(SeedCapturer* app) {
    if(!app->rx_active) {
        /* a failed start still allocated the receiver side, drop it */
        if(app->receiver) {
            subghz_receiver_free(app->receiver);
            app->receiver = NULL;
        }
        if(app->environment) {
            subghz_environment_free(app->environment);
            app->environment = NULL;
        }
        return;
    }
    if(app->worker && subghz_worker_is_running(app->worker)) subghz_worker_stop(app->worker);
    subghz_devices_stop_async_rx(app->device);
    subghz_devices_idle(app->device);
    subghz_devices_sleep(app->device);
    subghz_devices_end(app->device);
    subghz_devices_deinit();
    if(app->worker) {
        subghz_worker_free(app->worker);
        app->worker = NULL;
    }
    subghz_receiver_free(app->receiver);
    subghz_environment_free(app->environment);
    app->receiver = NULL;
    app->environment = NULL;
    app->rx_active = false;
}

static void seed_capture_reset(SeedCapturer* app) {
    app->has_fix = false;
    app->fix = 0;
    app->hop_count = 0;
    app->packets = 0;
    memset(app->hops, 0, sizeof(app->hops));
}

/* --------------------------------------------------------------------- files */

/* One file per remote, named so a folder full of them still says which remote and when
 * at a glance. Fix identifies the remote, the stamp keeps repeat sessions apart. */
static bool seed_save(SeedCapturer* app) {
    const SeedMfrInfo* mfr = &seed_mfr[app->mfr_index];
    const uint32_t frequency = seed_freq[app->freq_index].hz;

    storage_simply_mkdir(app->storage, SEED_PARENT);
    if(!storage_simply_mkdir(app->storage, SEED_FOLDER)) {
        seed_set_status(app, "Cannot create folder");
        return false;
    }

    char name[64];
    snprintf(
        name,
        sizeof(name),
        "%s_%08lX_%02u%02u%02u-%02u%02u%02u%s",
        mfr->prefix,
        app->fix,
        app->first_seen.year % 100u,
        app->first_seen.month,
        app->first_seen.day,
        app->first_seen.hour,
        app->first_seen.minute,
        app->first_seen.second,
        SEED_EXT);

    char path[SEED_PATH_LEN];
    snprintf(path, sizeof(path), "%s/%s", SEED_FOLDER, name);

    FlipperFormat* fff = flipper_format_file_alloc(app->storage);
    bool ok = false;
    do {
        if(!flipper_format_file_open_always(fff, path)) break;
        if(!flipper_format_write_header_cstr(fff, "Flipper SubGhz Seed Capture", 1)) break;
        if(!flipper_format_write_comment_cstr(
               fff, "Fix + Hop set for offline seed recovery, not a playable .sub"))
            break;

        char buf[32];
        snprintf(
            buf,
            sizeof(buf),
            "%04u-%02u-%02u %02u:%02u:%02u",
            app->first_seen.year,
            app->first_seen.month,
            app->first_seen.day,
            app->first_seen.hour,
            app->first_seen.minute,
            app->first_seen.second);
        if(!flipper_format_write_string_cstr(fff, "Received", buf)) break;

        if(!flipper_format_write_string_cstr(fff, "Manufacturer", mfr->label)) break;
        if(!flipper_format_write_string_cstr(fff, "Protocol", mfr->protocol)) break;
        if(!flipper_format_write_uint32(fff, "Frequency", &frequency, 1)) break;
        if(!flipper_format_write_string_cstr(fff, "Preset", SEED_PRESET)) break;

        snprintf(buf, sizeof(buf), "%08lX", app->fix);
        if(!flipper_format_write_string_cstr(fff, "Fix", buf)) break;

        uint32_t count = app->hop_count;
        if(!flipper_format_write_uint32(fff, "Hops", &count, 1)) break;
        if(!flipper_format_write_comment_cstr(fff, "Hops in the order they were received")) break;

        bool hops_ok = true;
        for(uint8_t i = 0; i < app->hop_count; i++) {
            snprintf(buf, sizeof(buf), "%08lX", app->hops[i]);
            if(!flipper_format_write_string_cstr(fff, "Hop", buf)) {
                hops_ok = false;
                break;
            }
        }
        ok = hops_ok;
    } while(false);

    flipper_format_free(fff);

    char msg[96];
    if(ok) {
        snprintf(msg, sizeof(msg), "Saved %u hops\n%s", app->hop_count, name);
    } else {
        snprintf(msg, sizeof(msg), "Save failed\n%s", name);
    }
    seed_set_status(app, msg);
    return ok;
}

static uint32_t seed_count_files(SeedCapturer* app) {
    uint32_t count = 0;
    File* dir = storage_file_alloc(app->storage);
    if(storage_dir_open(dir, SEED_FOLDER)) {
        FileInfo info;
        char name[64];
        while(storage_dir_read(dir, &info, name, sizeof(name))) {
            if(!file_info_is_dir(&info)) count++;
        }
    }
    storage_dir_close(dir);
    storage_file_free(dir);
    return count;
}

static bool seed_wipe(SeedCapturer* app) {
    const uint32_t count = seed_count_files(app);
    bool ok = storage_simply_remove_recursive(app->storage, SEED_FOLDER);
    if(ok) {
        storage_simply_mkdir(app->storage, SEED_PARENT);
        ok = storage_simply_mkdir(app->storage, SEED_FOLDER);
    }

    char msg[64];
    if(ok) {
        snprintf(msg, sizeof(msg), "Deleted %lu capture%s", count, (count == 1) ? "" : "s");
    } else {
        snprintf(msg, sizeof(msg), "Delete failed");
    }
    seed_set_status(app, msg);
    return ok;
}

/* ---------------------------------------------------------------------- main */

/* Leaves the capture screen, always dropping the radio first. */
static void seed_leave_capture(SeedCapturer* app, SeedScene next) {
    furi_mutex_release(app->mutex);
    seed_rx_stop(app);
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    app->scene = next;
}

int32_t seed_capturer_app(void* p) {
    UNUSED(p);

    SeedCapturer* app = malloc(sizeof(SeedCapturer));
    memset(app, 0, sizeof(SeedCapturer));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->queue = furi_message_queue_alloc(16, sizeof(InputEvent));
    app->storage = furi_record_open(RECORD_STORAGE);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->status = furi_string_alloc_set("Idle");
    app->scene = SeedSceneMenu;

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, seed_draw_callback, app);
    view_port_input_callback_set(app->view_port, seed_input_callback, app->queue);
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    bool running = true;
    InputEvent event;
    while(running) {
        if(furi_message_queue_get(app->queue, &event, 200) == FuriStatusOk) {
            if(event.type != InputTypeShort) {
                view_port_update(app->view_port);
                continue;
            }
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            switch(app->scene) {
            case SeedSceneMenu:
                if((event.key == InputKeyUp) && app->menu_index) app->menu_index--;
                if((event.key == InputKeyDown) && (app->menu_index < COUNT_OF(seed_menu) - 1))
                    app->menu_index++;
                if(event.key == InputKeyBack) running = false;
                if(event.key == InputKeyOk) {
                    if(app->menu_index == 0) {
                        app->scene = SeedSceneMfr;
                    } else if(app->menu_index == 1) {
                        app->scene = SeedSceneWipeConfirm;
                    } else {
                        running = false;
                    }
                }
                break;

            case SeedSceneMfr:
                if((event.key == InputKeyUp) && app->mfr_index) app->mfr_index--;
                if((event.key == InputKeyDown) && (app->mfr_index < SeedMfrCount - 1))
                    app->mfr_index++;
                if((event.key == InputKeyBack) || (event.key == InputKeyLeft))
                    app->scene = SeedSceneMenu;
                if(event.key == InputKeyOk) app->scene = SeedSceneFreq;
                break;

            case SeedSceneFreq:
                if((event.key == InputKeyUp) && app->freq_index) app->freq_index--;
                if((event.key == InputKeyDown) && (app->freq_index < COUNT_OF(seed_freq) - 1))
                    app->freq_index++;
                if((event.key == InputKeyBack) || (event.key == InputKeyLeft))
                    app->scene = SeedSceneMfr;
                if(event.key == InputKeyOk) {
                    seed_capture_reset(app);
                    furi_string_printf(app->status, "Press remote %u+ times", SEED_MIN_HOP);
                    app->scene = SeedSceneCapture;
                    furi_mutex_release(app->mutex);
                    const bool started = seed_rx_start(app);
                    if(!started) seed_rx_stop(app);
                    furi_mutex_acquire(app->mutex, FuriWaitForever);
                    if(!started) {
                        furi_string_set(app->status, "Radio unavailable");
                        app->scene = SeedSceneResult;
                        notification_message(app->notifications, &sequence_error);
                    }
                }
                break;

            case SeedSceneCapture:
                if(event.key == InputKeyBack) {
                    seed_leave_capture(app, SeedSceneMenu);
                } else if(event.key == InputKeyLeft) {
                    seed_capture_reset(app);
                    furi_string_set(app->status, "Reset, listening");
                } else if((event.key == InputKeyOk) && (app->hop_count >= SEED_MIN_HOP)) {
                    seed_leave_capture(app, SeedSceneResult);
                    furi_mutex_release(app->mutex);
                    const bool saved = seed_save(app);
                    furi_mutex_acquire(app->mutex, FuriWaitForever);
                    notification_message(
                        app->notifications, saved ? &sequence_success : &sequence_error);
                }
                break;

            case SeedSceneWipeConfirm:
                if((event.key == InputKeyBack) || (event.key == InputKeyLeft)) {
                    app->scene = SeedSceneMenu;
                } else if(event.key == InputKeyOk) {
                    app->scene = SeedSceneResult;
                    furi_mutex_release(app->mutex);
                    const bool wiped = seed_wipe(app);
                    furi_mutex_acquire(app->mutex, FuriWaitForever);
                    notification_message(
                        app->notifications, wiped ? &sequence_success : &sequence_error);
                }
                break;

            case SeedSceneResult:
                if((event.key == InputKeyBack) || (event.key == InputKeyLeft) ||
                   (event.key == InputKeyOk))
                    app->scene = SeedSceneMenu;
                break;
            }
            furi_mutex_release(app->mutex);
        }
        view_port_update(app->view_port);
    }

    seed_rx_stop(app);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_STORAGE);
    furi_message_queue_free(app->queue);
    furi_mutex_free(app->mutex);
    furi_string_free(app->status);
    free(app);
    return 0;
}
