/*
 * Nice O-Code — recover a remote's 16 bit installer code.
 *
 * Nice O-Code is Nice Flor-S whose final cipher permutation is XORed with a per remote
 * installer code instead of being inverted, plus a per parcel mask on the serial. A
 * single capture is not enough to tell you the code, but four or more parcels from the
 * same remote are: only the correct installer code makes every parcel decode to the same
 * serial. This app listens for parcels, brute forces the code, and writes a .sub with
 * the recovered "IC" key so the sub-ghz app can decode and replay that remote.
 */

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/elements.h>
#include <gui/modules/text_input.h>
#include <gui/view_holder.h>
#include <toolbox/name_generator.h>
#include <toolbox/path.h>

#include <lib/subghz/receiver.h>
#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/environment.h>
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/protocols/nice_flor_s.h>
#include <lib/subghz/protocols/public_api.h>
#include <lib/subghz/blocks/generic.h>
#include <lib/subghz/devices/devices.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>
#include <flipper_format/flipper_format.h>

#define TAG              "NiceOCode"
#define NICE_O_FOLDER    "/ext/subghz"
#define NICE_O_EXT       ".sub"
#define NICE_O_NAME_LEN  32
#define NICE_O_KEYSTORE  "/ext/subghz/assets/nice_flor_s"
#define NICE_O_MAX_KEYS  16
#define NICE_O_MIN_KEYS  4
#define NICE_O_FREQUENCY 433920000

#define NICE_O_SWEEP_TOTAL 0x10000u
/* candidates per main loop pass: small enough that the screen keeps redrawing,
 * large enough that the whole sweep is over in well under a second */
#define NICE_O_SWEEP_CHUNK 4096u

typedef enum {
    NiceOSceneMenu,
    NiceOSceneCapture,
    NiceOSceneSolve,
    NiceOSceneResult,
} NiceOScene;

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* queue;
    ViewPort* view_port;
    Gui* gui;
    Storage* storage;
    DialogsApp* dialogs;
    NotificationApp* notifications;

    NiceOScene scene;
    uint8_t menu_index;
    FuriString* status;

    // capture
    SubGhzEnvironment* environment;
    SubGhzReceiver* receiver;
    SubGhzWorker* worker;
    const SubGhzDevice* device;
    volatile bool rx_active;
    uint64_t keys[NICE_O_MAX_KEYS];
    uint8_t key_count;
    uint32_t packets;

    // sweep, run in chunks from the main loop so the UI never freezes
    uint32_t mask_32[NICE_O_MAX_KEYS];
    uint16_t mask_16[NICE_O_MAX_KEYS];
    uint8_t sweep_keys;
    uint32_t sweep_ic;
    bool sweep_found;
    bool sweep_ambiguous;
    uint16_t sweep_ic_found;
    uint32_t sweep_serial;

    // result
    bool solved;
    uint16_t ic;
    uint32_t serial;
} NiceOCode;

/* ------------------------------------------------------------------ drawing */

static void nice_o_draw_callback(Canvas* canvas, void* ctx) {
    NiceOCode* app = ctx;
    char buf[48];
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Nice O-Code");
    canvas_draw_line(canvas, 0, 13, 128, 13);
    canvas_set_font(canvas, FontSecondary);

    switch(app->scene) {
    case NiceOSceneMenu: {
        static const char* items[] = {"Capture remote", "Exit"};
        for(uint8_t i = 0; i < COUNT_OF(items); i++) {
            if(i == app->menu_index) {
                canvas_draw_box(canvas, 0, 17 + i * 12, 128, 11);
                canvas_set_color(canvas, ColorWhite);
            }
            canvas_draw_str(canvas, 4, 26 + i * 12, items[i]);
            canvas_set_color(canvas, ColorBlack);
        }
        elements_frame(canvas, 0, 41, 128, 23);
        canvas_draw_str(canvas, 4, 51, "Press the button 4+ times,");
        canvas_draw_str(canvas, 4, 62, "do not hold. 433.92 AM650");
        break;
    }

    case NiceOSceneCapture: {
        snprintf(
            buf,
            sizeof(buf),
            "Presses %u/%u   %lupkt",
            app->key_count,
            NICE_O_MIN_KEYS,
            app->packets);
        canvas_draw_str(canvas, 2, 24, buf);

        /* fills as the capture approaches the four packets the sweep needs */
        const uint8_t have = MIN(app->key_count, (uint8_t)NICE_O_MIN_KEYS);
        elements_progress_bar(canvas, 2, 28, 124, (float)have / (float)NICE_O_MIN_KEYS);

        elements_frame(canvas, 0, 40, 128, 13);
        canvas_draw_str(canvas, 4, 49, furi_string_get_cstr(app->status));

        if(app->key_count >= NICE_O_MIN_KEYS) {
            elements_button_center(canvas, "Solve");
        }
        elements_button_left(canvas, "Back");
        break;
    }

    case NiceOSceneSolve: {
        canvas_draw_str(canvas, 2, 26, "Trying installer codes");
        snprintf(buf, sizeof(buf), "%lu / %u", app->sweep_ic, NICE_O_SWEEP_TOTAL);
        elements_progress_bar_with_text(
            canvas, 2, 32, 124, (float)app->sweep_ic / (float)NICE_O_SWEEP_TOTAL, buf);
        elements_button_left(canvas, "Cancel");
        break;
    }

    default:
        if(app->solved) {
            elements_frame(canvas, 0, 17, 128, 30);
            canvas_set_font(canvas, FontPrimary);
            snprintf(buf, sizeof(buf), "IC  %04X", app->ic);
            canvas_draw_str(canvas, 6, 30, buf);
            canvas_set_font(canvas, FontSecondary);
            snprintf(buf, sizeof(buf), "Serial  %07lX", app->serial);
            canvas_draw_str(canvas, 6, 42, buf);
            elements_button_center(canvas, "Save");
        } else {
            elements_multiline_text_aligned(
                canvas, 64, 32, AlignCenter, AlignCenter, furi_string_get_cstr(app->status));
        }
        elements_button_left(canvas, "Back");
        break;
    }

    furi_mutex_release(app->mutex);
}

static void nice_o_input_callback(InputEvent* event, void* ctx) {
    FuriMessageQueue* queue = ctx;
    furi_message_queue_put(queue, event, FuriWaitForever);
}

/* ------------------------------------------------------------------ capture */

/*
 * Every frame of a single press carries the same counter and only rotates the parcel
 * index. The sweep XORs the parcel mask into the trial IC on both the encrypt and the
 * decrypt side, so it cancels: same counter frames agree on the recovered serial for
 * EVERY candidate IC and rule nothing out. Holding the button therefore yields any
 * number of packets and still leaves all 65536 codes standing.
 *
 * Two probe ICs are enough to tell the two cases apart: if a new frame agrees with one
 * already held at both probes, it is the same press.
 */
static bool nice_o_key_adds_info(NiceOCode* app, uint64_t key) {
    static const uint16_t probe[2] = {0x0000u, 0x0001u};

    uint32_t mask_32_new = 0;
    uint16_t mask_16_new = 0;
    subghz_protocol_nice_o_mask(
        subghz_protocol_nice_o_get_parcel(key), &mask_32_new, &mask_16_new);

    for(uint8_t i = 0; i < app->key_count; i++) {
        if(app->keys[i] == key) return false;

        uint32_t mask_32_old = 0;
        uint16_t mask_16_old = 0;
        subghz_protocol_nice_o_mask(
            subghz_protocol_nice_o_get_parcel(app->keys[i]), &mask_32_old, &mask_16_old);

        bool same_press = true;
        for(uint8_t p = 0; p < COUNT_OF(probe); p++) {
            const uint64_t a = subghz_protocol_nice_flor_s_decrypt_ic(
                key, (uint16_t)(probe[p] ^ mask_16_new), NICE_O_KEYSTORE);
            const uint64_t b = subghz_protocol_nice_flor_s_decrypt_ic(
                app->keys[i], (uint16_t)(probe[p] ^ mask_16_old), NICE_O_KEYSTORE);
            if((((uint32_t)(a >> 16) ^ mask_32_new) & 0x0FFFFFFFu) !=
               (((uint32_t)(b >> 16) ^ mask_32_old) & 0x0FFFFFFFu)) {
                same_press = false;
                break;
            }
        }
        if(same_press) return false;
    }
    return true;
}

/* Runs on the SubGhzWorker thread, so allocating here is fine. */
static void
    nice_o_rx_callback(SubGhzReceiver* receiver, SubGhzProtocolDecoderBase* base, void* ctx) {
    UNUSED(receiver);
    NiceOCode* app = ctx;

    if(strcmp(base->protocol->name, SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME) != 0) return;

    // pull the raw key out through the public serializer instead of poking at internals
    uint64_t key = 0;
    FlipperFormat* fff = flipper_format_string_alloc();
    SubGhzRadioPreset preset = {
        .name = furi_string_alloc_set("FuriHalSubGhzPresetOok650Async"),
        .frequency = NICE_O_FREQUENCY,
        .data = NULL,
        .data_size = 0,
    };
    uint8_t key_data[8] = {0};
    if((subghz_protocol_decoder_base_serialize(base, fff, &preset) == SubGhzProtocolStatusOk) &&
       flipper_format_rewind(fff) &&
       flipper_format_read_hex(fff, "Key", key_data, sizeof(key_data))) {
        for(size_t i = 0; i < sizeof(key_data); i++) {
            key = (key << 8) | key_data[i];
        }
    }
    furi_string_free(preset.name);
    flipper_format_free(fff);
    if(key == 0) return;

    furi_mutex_acquire(app->mutex, FuriWaitForever);
    app->packets++;
    if((app->key_count < NICE_O_MAX_KEYS) && nice_o_key_adds_info(app, key)) {
        app->keys[app->key_count++] = key;
        furi_string_printf(
            app->status, "Press %u: %010llX", app->key_count, key & 0xFFFFFFFFFFULL);
        notification_message(app->notifications, &sequence_blink_green_10);
    } else {
        furi_string_printf(app->status, "Same press, ignored");
    }
    furi_mutex_release(app->mutex);
}

static bool nice_o_rx_start(NiceOCode* app) {
    app->environment = subghz_environment_alloc();
    subghz_environment_set_protocol_registry(app->environment, (void*)&subghz_protocol_registry);
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(app->environment, NICE_O_KEYSTORE);

    app->receiver = subghz_receiver_alloc_init(app->environment);
    subghz_receiver_set_filter(app->receiver, SubGhzProtocolFlag_Decodable);
    subghz_receiver_set_rx_callback(app->receiver, nice_o_rx_callback, app);

    /* The decoder hides O-Code frames from normal receiving, since without an installer
     * code they decode to a random serial. This app is the thing that finds that code,
     * so it needs them. */
    SubGhzProtocolDecoderBase* nice = subghz_receiver_search_decoder_base_by_name(
        app->receiver, SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME);
    if(nice) subghz_protocol_decoder_nice_flor_s_set_skip_o_code(nice, false);

    subghz_devices_init();
    app->device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);
    if(!app->device) return false;

    subghz_devices_begin(app->device);
    subghz_devices_reset(app->device);
    subghz_devices_load_preset(app->device, FuriHalSubGhzPresetOok650Async, NULL);
    subghz_devices_set_frequency(app->device, NICE_O_FREQUENCY);
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

static void nice_o_rx_stop(NiceOCode* app) {
    if(!app->rx_active) return;
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

/* -------------------------------------------------------------------- solve */

/* Set up a sweep over all 65536 installer codes. The rainbow table is read once
 * here: reading it opens and decrypts a file on the SD card, so doing that per
 * candidate would turn a sub second search into one that never seems to end. */
static bool nice_o_solve_begin(NiceOCode* app) {
    if(app->key_count < NICE_O_MIN_KEYS) {
        furi_string_set(app->status, "Need 4+ packets");
        return false;
    }
    /* warm the lib side table cache once so the sweep never touches the card */
    if(subghz_protocol_nice_flor_s_decrypt_ic(app->keys[0], 0, NICE_O_KEYSTORE) ==
       SUBGHZ_NO_NICE_FLOR_S_RAINBOW_TABLE) {
        furi_string_set(app->status, "No rainbow table");
        return false;
    }

    app->sweep_keys = app->key_count;
    for(uint8_t i = 0; i < app->sweep_keys; i++) {
        subghz_protocol_nice_o_mask(
            subghz_protocol_nice_o_get_parcel(app->keys[i]), &app->mask_32[i], &app->mask_16[i]);
    }
    app->sweep_ic = 0;
    app->sweep_found = false;
    app->sweep_ambiguous = false;
    return true;
}

/* Try one chunk of candidates. Returns true once the whole range has been swept. */
static bool nice_o_solve_step(NiceOCode* app) {
    const uint32_t end = MIN(app->sweep_ic + NICE_O_SWEEP_CHUNK, NICE_O_SWEEP_TOTAL);

    for(; app->sweep_ic < end; app->sweep_ic++) {
        const uint16_t ic = (uint16_t)app->sweep_ic;
        uint64_t d = subghz_protocol_nice_flor_s_decrypt_ic(
            app->keys[0], (uint16_t)(ic ^ app->mask_16[0]), NICE_O_KEYSTORE);
        const uint32_t serial = (uint32_t)((d >> 16) ^ app->mask_32[0]) & 0x0FFFFFFFu;

        uint8_t i = 1;
        for(; i < app->sweep_keys; i++) {
            d = subghz_protocol_nice_flor_s_decrypt_ic(
                app->keys[i], (uint16_t)(ic ^ app->mask_16[i]), NICE_O_KEYSTORE);
            if(((uint32_t)((d >> 16) ^ app->mask_32[i]) & 0x0FFFFFFFu) != serial) break;
        }
        if(i != app->sweep_keys) continue;

        /* a second hit means the captures cannot tell the codes apart */
        if(app->sweep_found) {
            app->sweep_ambiguous = true;
            app->sweep_ic = NICE_O_SWEEP_TOTAL;
            return true;
        }
        app->sweep_found = true;
        app->sweep_ic_found = ic;
        app->sweep_serial = serial;
    }
    return app->sweep_ic >= NICE_O_SWEEP_TOTAL;
}

static void nice_o_solve_finish(NiceOCode* app) {
    if(app->sweep_ambiguous) {
        app->solved = false;
        furi_string_set(app->status, "Ambiguous, capture more");
    } else if(app->sweep_found) {
        app->solved = true;
        app->ic = app->sweep_ic_found;
        app->serial = app->sweep_serial;
        furi_string_set(app->status, "Found");
    } else {
        app->solved = false;
        furi_string_set(app->status, "No code fits, not O-Code?");
    }
}

typedef struct {
    FuriSemaphore* done;
    bool accepted;
} NiceONameCtx;

static void nice_o_name_done(void* context) {
    NiceONameCtx* ctx = context;
    ctx->accepted = true;
    furi_semaphore_release(ctx->done);
}

static void nice_o_name_back(void* context) {
    NiceONameCtx* ctx = context;
    ctx->accepted = false;
    furi_semaphore_release(ctx->done);
}

/* Hands the screen to the keyboard, blocks until the name is confirmed or dropped,
 * then gives the screen back. Returns false if the user backed out. */
static bool nice_o_ask_name(NiceOCode* app, char* name, size_t name_size) {
    gui_remove_view_port(app->gui, app->view_port);

    NiceONameCtx ctx = {.done = furi_semaphore_alloc(1, 0), .accepted = false};
    TextInput* text_input = text_input_alloc();
    text_input_set_header_text(text_input, "Name the .sub file");
    text_input_set_result_callback(text_input, nice_o_name_done, &ctx, name, name_size, false);

    ValidatorIsFile* validator = validator_is_file_alloc_init(NICE_O_FOLDER, NICE_O_EXT, NULL);
    text_input_set_validator(text_input, validator_is_file_callback, validator);

    ViewHolder* view_holder = view_holder_alloc();
    view_holder_attach_to_gui(view_holder, app->gui);
    view_holder_set_back_callback(view_holder, nice_o_name_back, &ctx);
    view_holder_set_view(view_holder, text_input_get_view(text_input));

    furi_semaphore_acquire(ctx.done, FuriWaitForever);

    view_holder_set_view(view_holder, NULL);
    view_holder_free(view_holder);
    text_input_free(text_input);
    validator_is_file_free(validator);
    furi_semaphore_free(ctx.done);

    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    return ctx.accepted;
}

/* Status is read by the draw callback on the GUI thread, so every write takes the
 * mutex. It is never held across nice_o_ask_name(), which blocks on the keyboard. */
static void nice_o_set_status(NiceOCode* app, const char* text) {
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    furi_string_set(app->status, text);
    furi_mutex_release(app->mutex);
}

static bool nice_o_save(NiceOCode* app) {
    char name[NICE_O_NAME_LEN];
    name_generator_make_random_prefixed(name, sizeof(name), "NiceO");
    if(!nice_o_ask_name(app, name, sizeof(name))) {
        nice_o_set_status(app, "Save cancelled");
        return false;
    }

    FuriString* path = furi_string_alloc();
    furi_string_printf(path, "%s/%s%s", NICE_O_FOLDER, name, NICE_O_EXT);

    FlipperFormat* fff = flipper_format_file_alloc(app->storage);
    bool ok = false;
    do {
        if(!flipper_format_file_open_always(fff, furi_string_get_cstr(path))) break;
        if(!flipper_format_write_header_cstr(fff, "Flipper SubGhz Key File", 1)) break;
        uint32_t freq = NICE_O_FREQUENCY;
        if(!flipper_format_write_uint32(fff, "Frequency", &freq, 1)) break;
        if(!flipper_format_write_string_cstr(fff, "Preset", "FuriHalSubGhzPresetOok650Async"))
            break;
        if(!flipper_format_write_string_cstr(fff, "Protocol", SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME))
            break;
        uint32_t bits = 52;
        if(!flipper_format_write_uint32(fff, "Bit", &bits, 1)) break;

        uint8_t key_data[8];
        for(size_t i = 0; i < 8; i++) {
            key_data[7 - i] = (app->keys[0] >> (i * 8)) & 0xFF;
        }
        if(!flipper_format_write_hex(fff, "Key", key_data, sizeof(key_data))) break;

        uint8_t ic_data[2] = {(uint8_t)(app->ic >> 8), (uint8_t)app->ic};
        if(!flipper_format_write_hex(fff, "IC", ic_data, sizeof(ic_data))) break;
        ok = true;
    } while(false);

    flipper_format_free(fff);
    char msg[NICE_O_NAME_LEN + 16];
    snprintf(msg, sizeof(msg), ok ? "Saved %s%s" : "Save failed: %s%s", name, NICE_O_EXT);
    nice_o_set_status(app, msg);
    furi_string_free(path);
    return ok;
}

/* ---------------------------------------------------------------------- main */

int32_t nice_o_code_app(void* p) {
    UNUSED(p);

    NiceOCode* app = malloc(sizeof(NiceOCode));
    memset(app, 0, sizeof(NiceOCode));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->queue = furi_message_queue_alloc(16, sizeof(InputEvent));
    app->storage = furi_record_open(RECORD_STORAGE);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->status = furi_string_alloc_set("Idle");
    app->scene = NiceOSceneMenu;

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, nice_o_draw_callback, app);
    view_port_input_callback_set(app->view_port, nice_o_input_callback, app->queue);
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    bool running = true;
    InputEvent event;
    while(running) {
        /* the sweep runs a chunk at a time here rather than in one blocking call, so
         * the progress bar keeps moving and Back still cancels */
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        const bool solving = (app->scene == NiceOSceneSolve);
        if(solving) {
            if(nice_o_solve_step(app)) {
                nice_o_solve_finish(app);
                app->scene = NiceOSceneResult;
                notification_message(
                    app->notifications, app->solved ? &sequence_success : &sequence_error);
            }
        }
        furi_mutex_release(app->mutex);
        if(solving) view_port_update(app->view_port);

        if(furi_message_queue_get(app->queue, &event, solving ? 0 : 200) == FuriStatusOk) {
            if(event.type == InputTypeShort) {
                furi_mutex_acquire(app->mutex, FuriWaitForever);
                switch(app->scene) {
                case NiceOSceneMenu:
                    if(event.key == InputKeyUp && app->menu_index) app->menu_index--;
                    if(event.key == InputKeyDown && app->menu_index < 1) app->menu_index++;
                    if(event.key == InputKeyBack) running = false;
                    if(event.key == InputKeyOk) {
                        if(app->menu_index == 1) {
                            running = false;
                        } else {
                            app->key_count = 0;
                            app->packets = 0;
                            app->solved = false;
                            furi_string_set(app->status, "Listening 433.92");
                            app->scene = NiceOSceneCapture;
                            furi_mutex_release(app->mutex);
                            if(!nice_o_rx_start(app)) {
                                furi_mutex_acquire(app->mutex, FuriWaitForever);
                                furi_string_set(app->status, "Radio busy");
                                app->scene = NiceOSceneMenu;
                                furi_mutex_release(app->mutex);
                            }
                            furi_mutex_acquire(app->mutex, FuriWaitForever);
                        }
                    }
                    break;

                case NiceOSceneCapture:
                    if((event.key == InputKeyBack) || (event.key == InputKeyLeft)) {
                        furi_mutex_release(app->mutex);
                        nice_o_rx_stop(app);
                        furi_mutex_acquire(app->mutex, FuriWaitForever);
                        app->scene = NiceOSceneMenu;
                    } else if((event.key == InputKeyOk) && (app->key_count >= NICE_O_MIN_KEYS)) {
                        furi_mutex_release(app->mutex);
                        nice_o_rx_stop(app);
                        furi_mutex_acquire(app->mutex, FuriWaitForever);
                        if(nice_o_solve_begin(app)) {
                            app->scene = NiceOSceneSolve;
                        } else {
                            app->solved = false;
                            app->scene = NiceOSceneResult;
                            notification_message(app->notifications, &sequence_error);
                        }
                    }
                    break;

                case NiceOSceneSolve:
                    if((event.key == InputKeyBack) || (event.key == InputKeyLeft)) {
                        furi_string_set(app->status, "Cancelled");
                        app->solved = false;
                        app->scene = NiceOSceneResult;
                    }
                    break;

                case NiceOSceneResult:
                    if((event.key == InputKeyBack) || (event.key == InputKeyLeft)) {
                        app->scene = NiceOSceneMenu;
                    } else if((event.key == InputKeyOk) && app->solved) {
                        furi_mutex_release(app->mutex);
                        const bool saved = nice_o_save(app);
                        furi_mutex_acquire(app->mutex, FuriWaitForever);
                        notification_message(
                            app->notifications, saved ? &sequence_success : &sequence_error);
                    }
                    break;
                }
                furi_mutex_release(app->mutex);
            }
        }
        view_port_update(app->view_port);
    }

    nice_o_rx_stop(app);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);
    furi_message_queue_free(app->queue);
    furi_mutex_free(app->mutex);
    furi_string_free(app->status);
    free(app);
    return 0;
}
