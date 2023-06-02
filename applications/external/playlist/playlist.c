#include <furi.h>

#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

#include <lib/toolbox/path.h>
#include <subghz_playlist_icons.h>

#include <lib/subghz/protocols/protocol_items.h>
#include <flipper_format/flipper_format_i.h>
#include <applications/main/subghz/subghz_i.h>

#include "flipper_format_stream.h"
#include "flipper_format_stream_i.h"

#include <lib/subghz/transmitter.h>
#include <lib/subghz/protocols/raw.h>

#include "playlist_file.h"
#include "canvas_helper.h"

#define PLAYLIST_FOLDER "/ext/subplaylist"
#define PLAYLIST_EXT ".txt"
#define TAG "Playlist"

#define STATE_NONE 0
#define STATE_OVERVIEW 1
#define STATE_SENDING 2

#define WIDTH 128
#define HEIGHT 64

typedef struct {
    int current_count; // number of processed files
    int total_count; // number of items in the playlist

    int playlist_repetitions; // number of times to repeat the whole playlist
    int current_playlist_repetition; // current playlist repetition

    // last 3 files
    FuriString* prev_0_path; // current file
    FuriString* prev_1_path; // previous file
    FuriString* prev_2_path; // previous previous file
    FuriString* prev_3_path; // you get the idea

    int state; // current state

    ViewPort* view_port;
} DisplayMeta;

typedef struct {
    FuriThread* thread;
    Storage* storage;
    FlipperFormat* format;

    DisplayMeta* meta;

    FuriString* file_path; // path to the playlist file

    bool ctl_request_exit; // can be set to true if the worker should exit
    bool ctl_pause; // can be set to true if the worker should pause
    bool ctl_request_skip; // can be set to true if the worker should skip the current file
    bool ctl_request_prev; // can be set to true if the worker should go to the previous file

    bool is_running; // indicates if the worker is running
} PlaylistWorker;

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    Gui* gui;

    DisplayMeta* meta;
    PlaylistWorker* worker;

    FuriString* file_path; // Path to the playlist file
} Playlist;

////////////////////////////////////////////////////////////////////////////////

void meta_set_state(DisplayMeta* meta, int state) {
    meta->state = state;
    view_port_update(meta->view_port);
}

static FuriHalSubGhzPreset str_to_preset(FuriString* preset) {
    if(furi_string_cmp_str(preset, "FuriHalSubGhzPresetOok270Async") == 0) {
        return FuriHalSubGhzPresetOok270Async;
    }
    if(furi_string_cmp_str(preset, "FuriHalSubGhzPresetOok650Async") == 0) {
        return FuriHalSubGhzPresetOok650Async;
    }
    if(furi_string_cmp_str(preset, "FuriHalSubGhzPreset2FSKDev238Async") == 0) {
        return FuriHalSubGhzPreset2FSKDev238Async;
    }
    if(furi_string_cmp_str(preset, "FuriHalSubGhzPreset2FSKDev476Async") == 0) {
        return FuriHalSubGhzPreset2FSKDev476Async;
    }
    if(furi_string_cmp_str(preset, "FuriHalSubGhzPresetMSK99_97KbAsync") == 0) {
        return FuriHalSubGhzPresetMSK99_97KbAsync;
    }
    if(furi_string_cmp_str(preset, "FuriHalSubGhzPresetMSK99_97KbAsync") == 0) {
        return FuriHalSubGhzPresetMSK99_97KbAsync;
    }
    return FuriHalSubGhzPresetCustom;
}

// -4: missing protocol
// -3: missing preset
// -2: transmit error
// -1: error
// 0: ok
// 1: resend
// 2: exited
static int playlist_worker_process(
    PlaylistWorker* worker,
    FlipperFormat* fff_file,
    FlipperFormat* fff_data,
    const char* path,
    FuriString* preset,
    FuriString* protocol) {
    // actual sending of .sub file

    if(!flipper_format_file_open_existing(fff_file, path)) {
        FURI_LOG_E(TAG, "  (TX) Failed to open %s", path);
        return -1;
    }

    // read frequency or default to 433.92MHz
    uint32_t frequency = 0;
    if(!flipper_format_read_uint32(fff_file, "Frequency", &frequency, 1)) {
        FURI_LOG_W(TAG, "  (TX) Missing Frequency, defaulting to 433.92MHz");
        frequency = 433920000;
    }
    if(!furi_hal_subghz_is_tx_allowed(frequency)) {
        return -2;
    }

    // check if preset is present
    if(!flipper_format_read_string(fff_file, "Preset", preset)) {
        FURI_LOG_E(TAG, "  (TX) Missing Preset");
        return -3;
    }

    // check if protocol is present
    if(!flipper_format_read_string(fff_file, "Protocol", protocol)) {
        FURI_LOG_E(TAG, "  (TX) Missing Protocol");
        return -4;
    }

    if(!furi_string_cmp_str(protocol, "RAW")) {
        subghz_protocol_raw_gen_fff_data(fff_data, path);
    } else {
        stream_copy_full(
            flipper_format_get_raw_stream(fff_file), flipper_format_get_raw_stream(fff_data));
    }
    flipper_format_free(fff_file);

    // (try to) send file
    SubGhzEnvironment* environment = subghz_environment_alloc();
    subghz_environment_set_protocol_registry(environment, (void*)&subghz_protocol_registry);
    SubGhzTransmitter* transmitter =
        subghz_transmitter_alloc_init(environment, furi_string_get_cstr(protocol));

    subghz_transmitter_deserialize(transmitter, fff_data);

    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(str_to_preset(preset));

    frequency = furi_hal_subghz_set_frequency_and_path(frequency);

    FURI_LOG_D(TAG, "  (TX) Start sending ...");
    int status = 0;

    furi_hal_subghz_start_async_tx(subghz_transmitter_yield, transmitter);
    while(!furi_hal_subghz_is_async_tx_complete()) {
        if(worker->ctl_request_exit) {
            FURI_LOG_D(TAG, "    (TX) Requested to exit. Cancelling sending...");
            status = 2;
            break;
        }
        if(worker->ctl_pause) {
            FURI_LOG_D(TAG, "    (TX) Requested to pause. Cancelling and resending...");
            status = 1;
            break;
        }
        if(worker->ctl_request_skip) {
            worker->ctl_request_skip = false;
            FURI_LOG_D(TAG, "    (TX) Requested to skip. Cancelling and resending...");
            status = 0;
            break;
        }
        if(worker->ctl_request_prev) {
            worker->ctl_request_prev = false;
            FURI_LOG_D(TAG, "    (TX) Requested to prev. Cancelling and resending...");
            status = 3;
            break;
        }
        furi_delay_ms(50);
    }

    FURI_LOG_D(TAG, "  (TX) Done sending.");

    furi_hal_subghz_stop_async_tx();
    furi_hal_subghz_sleep();

    subghz_transmitter_free(transmitter);

    return status;
}

// true - the worker can continue
// false - the worker should exit
static bool playlist_worker_wait_pause(PlaylistWorker* worker) {
    // wait if paused
    while(worker->ctl_pause && !worker->ctl_request_exit) {
        furi_delay_ms(50);
    }
    // exit loop if requested to stop
    if(worker->ctl_request_exit) {
        FURI_LOG_D(TAG, "Requested to exit. Exiting loop...");
        return false;
    }
    return true;
}

void updatePlayListView(PlaylistWorker* worker, const char* str) {
    furi_string_reset(worker->meta->prev_3_path);
    furi_string_set(worker->meta->prev_3_path, furi_string_get_cstr(worker->meta->prev_2_path));

    furi_string_reset(worker->meta->prev_2_path);
    furi_string_set(worker->meta->prev_2_path, furi_string_get_cstr(worker->meta->prev_1_path));

    furi_string_reset(worker->meta->prev_1_path);
    furi_string_set(worker->meta->prev_1_path, furi_string_get_cstr(worker->meta->prev_0_path));

    furi_string_reset(worker->meta->prev_0_path);
    furi_string_set(worker->meta->prev_0_path, str);

    view_port_update(worker->meta->view_port);
}

static bool playlist_worker_play_playlist_once(
    PlaylistWorker* worker,
    Storage* storage,
    FlipperFormat* fff_head,
    FlipperFormat* fff_data,
    FuriString* data,
    FuriString* preset,
    FuriString* protocol) {
    //
    if(!flipper_format_rewind(fff_head)) {
        FURI_LOG_E(TAG, "Failed to rewind file");
        return false;
    }

    while(flipper_format_read_string(fff_head, "sub", data)) {
        if(!playlist_worker_wait_pause(worker)) {
            break;
        }

        // update state to sending
        meta_set_state(worker->meta, STATE_SENDING);

        ++worker->meta->current_count;
        const char* str = furi_string_get_cstr(data);

        // it's not fancy, but it works for now :)
        updatePlayListView(worker, str);

        for(int i = 0; i < 1; i++) {
            if(!playlist_worker_wait_pause(worker)) {
                break;
            }

            view_port_update(worker->meta->view_port);

            FURI_LOG_D(TAG, "(worker) Sending %s", str);

            FlipperFormat* fff_file = flipper_format_file_alloc(storage);

            int status =
                playlist_worker_process(worker, fff_file, fff_data, str, preset, protocol);

            // if there was an error, fff_file is not already freed
            if(status < 0) {
                flipper_format_free(fff_file);
            }

            // re-send file is paused mid-send
            if(status == 1) {
                i -= 1;
                // errored, skip to next file
            } else if(status < 0) {
                break;
                // exited, exit loop
            } else if(status == 2) {
                return false;
            } else if(status == 3) {
                //aqui rebobinamos y avanzamos de nuevo el fichero n-1 veces
                //decrementamos el contador de ficheros enviados
                worker->meta->current_count--;
                if(worker->meta->current_count > 0) {
                    worker->meta->current_count--;
                }
                //rebobinamos el fichero
                if(!flipper_format_rewind(fff_head)) {
                    FURI_LOG_E(TAG, "Failed to rewind file");
                    return false;
                }
                //avanzamos el fichero n-1 veces
                for(int j = 0; j < worker->meta->current_count; j++) {
                    flipper_format_read_string(fff_head, "sub", data);
                }
                break;
            }
        }
    } // end of loop
    return true;
}

static int32_t playlist_worker_thread(void* ctx) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_head = flipper_format_file_alloc(storage);

    PlaylistWorker* worker = ctx;
    if(!flipper_format_file_open_existing(fff_head, furi_string_get_cstr(worker->file_path))) {
        FURI_LOG_E(TAG, "Failed to open %s", furi_string_get_cstr(worker->file_path));
        worker->is_running = false;

        furi_record_close(RECORD_STORAGE);
        flipper_format_free(fff_head);
        return 0;
    }

    playlist_worker_wait_pause(worker);
    FlipperFormat* fff_data = flipper_format_string_alloc();

    FuriString* data;
    FuriString* preset;
    FuriString* protocol;
    data = furi_string_alloc();
    preset = furi_string_alloc();
    protocol = furi_string_alloc();

    for(int i = 0; i < MAX(1, worker->meta->playlist_repetitions); i++) {
        // infinite repetitions if playlist_repetitions is 0
        if(worker->meta->playlist_repetitions <= 0) {
            --i;
        }
        ++worker->meta->current_playlist_repetition;
        // send playlist
        worker->meta->current_count = 0;
        if(worker->ctl_request_exit) {
            break;
        }

        FURI_LOG_D(
            TAG,
            "Sending playlist (i %d rep %d b %d)",
            i,
            worker->meta->current_playlist_repetition,
            worker->meta->playlist_repetitions);

        if(!playlist_worker_play_playlist_once(
               worker, storage, fff_head, fff_data, data, preset, protocol)) {
            break;
        }
    }

    furi_record_close(RECORD_STORAGE);
    flipper_format_free(fff_head);

    furi_string_free(data);
    furi_string_free(preset);
    furi_string_free(protocol);

    flipper_format_free(fff_data);

    FURI_LOG_D(TAG, "Done reading. Read %d data lines.", worker->meta->current_count);
    worker->is_running = false;

    // update state to overview
    meta_set_state(worker->meta, STATE_OVERVIEW);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

void playlist_meta_reset(DisplayMeta* instance) {
    instance->current_count = 0;
    instance->current_playlist_repetition = 0;

    furi_string_reset(instance->prev_0_path);
    furi_string_reset(instance->prev_1_path);
    furi_string_reset(instance->prev_2_path);
    furi_string_reset(instance->prev_3_path);
}

DisplayMeta* playlist_meta_alloc() {
    DisplayMeta* instance = malloc(sizeof(DisplayMeta));
    instance->prev_0_path = furi_string_alloc();
    instance->prev_1_path = furi_string_alloc();
    instance->prev_2_path = furi_string_alloc();
    instance->prev_3_path = furi_string_alloc();
    playlist_meta_reset(instance);
    instance->state = STATE_NONE;
    instance->playlist_repetitions = 1;
    return instance;
}

void playlist_meta_free(DisplayMeta* instance) {
    furi_string_free(instance->prev_0_path);
    furi_string_free(instance->prev_1_path);
    furi_string_free(instance->prev_2_path);
    furi_string_free(instance->prev_3_path);
    free(instance);
}

////////////////////////////////////////////////////////////////////////////////

PlaylistWorker* playlist_worker_alloc(DisplayMeta* meta) {
    PlaylistWorker* instance = malloc(sizeof(PlaylistWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "PlaylistWorker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, playlist_worker_thread);

    instance->meta = meta;
    instance->ctl_pause = true; // require the user to manually start the worker

    instance->file_path = furi_string_alloc();

    return instance;
}

void playlist_worker_free(PlaylistWorker* instance) {
    furi_assert(instance);
    furi_thread_free(instance->thread);
    furi_string_free(instance->file_path);
    free(instance);
}

void playlist_worker_stop(PlaylistWorker* worker) {
    furi_assert(worker);
    furi_assert(worker->is_running);

    worker->ctl_request_exit = true;
    furi_thread_join(worker->thread);
}

bool playlist_worker_running(PlaylistWorker* worker) {
    furi_assert(worker);
    return worker->is_running;
}

void playlist_worker_start(PlaylistWorker* instance, const char* file_path) {
    furi_assert(instance);
    furi_assert(!instance->is_running);

    furi_string_set(instance->file_path, file_path);
    instance->is_running = true;

    // reset meta (current/total)
    playlist_meta_reset(instance->meta);

    FURI_LOG_D(TAG, "Starting thread...");
    furi_thread_start(instance->thread);
}

////////////////////////////////////////////////////////////////////////////////

static void render_callback(Canvas* canvas, void* ctx) {
    Playlist* app = ctx;
    furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    FuriString* temp_str;
    temp_str = furi_string_alloc();

    switch(app->meta->state) {
    case STATE_NONE:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, WIDTH / 2, HEIGHT / 2, AlignCenter, AlignCenter, "No playlist loaded");
        break;

    case STATE_OVERVIEW:
        // draw file name
        {
            path_extract_filename(app->file_path, temp_str, true);

            canvas_set_font(canvas, FontPrimary);
            draw_centered_boxed_str(canvas, 1, 1, 15, 6, furi_string_get_cstr(temp_str));
        }

        canvas_set_font(canvas, FontSecondary);

        // draw loaded count
        {
            furi_string_printf(temp_str, "%d Items in playlist", app->meta->total_count);
            canvas_draw_str_aligned(
                canvas, 1, 19, AlignLeft, AlignTop, furi_string_get_cstr(temp_str));

            if(app->meta->playlist_repetitions <= 0) {
                furi_string_set(temp_str, "Repeat: inf");
            } else if(app->meta->playlist_repetitions == 1) {
                furi_string_set(temp_str, "Repeat: no");
            } else {
                furi_string_printf(temp_str, "Repeat: %dx", app->meta->playlist_repetitions);
            }
            canvas_draw_str_aligned(
                canvas, 1, 29, AlignLeft, AlignTop, furi_string_get_cstr(temp_str));
        }

        // draw buttons
        draw_corner_aligned(canvas, 40, 15, AlignCenter, AlignBottom);

        canvas_set_color(canvas, ColorWhite);
        canvas_draw_str_aligned(canvas, WIDTH / 2 - 7, HEIGHT - 11, AlignLeft, AlignTop, "Start");
        canvas_draw_disc(canvas, WIDTH / 2 - 14, HEIGHT - 8, 3);

        //
        canvas_set_color(canvas, ColorBlack);
        draw_corner_aligned(canvas, 20, 15, AlignLeft, AlignBottom);

        canvas_set_color(canvas, ColorWhite);
        canvas_draw_str_aligned(canvas, 4, HEIGHT - 11, AlignLeft, AlignTop, "R-");

        //
        canvas_set_color(canvas, ColorBlack);
        draw_corner_aligned(canvas, 20, 15, AlignRight, AlignBottom);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_str_aligned(canvas, WIDTH - 4, HEIGHT - 11, AlignRight, AlignTop, "R+");

        canvas_set_color(canvas, ColorBlack);

        break;
    case STATE_SENDING:
        canvas_set_color(canvas, ColorBlack);
        if(app->worker->ctl_pause) {
            canvas_draw_icon(canvas, 2, HEIGHT - 8, &I_ButtonRight_4x7);
        } else {
            canvas_draw_box(canvas, 2, HEIGHT - 8, 2, 7);
            canvas_draw_box(canvas, 5, HEIGHT - 8, 2, 7);
        }

        // draw progress text
        {
            canvas_set_font(canvas, FontSecondary);
            furi_string_printf(
                temp_str, "[%d/%d]", app->meta->current_count, app->meta->total_count);
            canvas_draw_str_aligned(
                canvas, 11, HEIGHT - 8, AlignLeft, AlignTop, furi_string_get_cstr(temp_str));

            int h = canvas_string_width(canvas, furi_string_get_cstr(temp_str));
            int xs = 11 + h + 2;
            int w = WIDTH - xs - 1;
            canvas_draw_box(canvas, xs, HEIGHT - 5, w, 1);

            float progress = (float)app->meta->current_count / (float)app->meta->total_count;
            int wp = (int)(progress * w);
            canvas_draw_box(canvas, xs + wp - 1, HEIGHT - 7, 2, 5);
        }

        {
            if(app->meta->playlist_repetitions <= 0) {
                furi_string_printf(temp_str, "[%d/Inf]", app->meta->current_playlist_repetition);
            } else {
                furi_string_printf(
                    temp_str,
                    "[%d/%d]",
                    app->meta->current_playlist_repetition,
                    app->meta->playlist_repetitions);
            }
            canvas_set_color(canvas, ColorBlack);
            int w = canvas_string_width(canvas, furi_string_get_cstr(temp_str));
            draw_corner_aligned(canvas, w + 6, 13, AlignRight, AlignTop);
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_str_aligned(
                canvas, WIDTH - 3, 3, AlignRight, AlignTop, furi_string_get_cstr(temp_str));
        }

        // draw last and current file
        {
            canvas_set_color(canvas, ColorBlack);
            canvas_set_font(canvas, FontSecondary);

            // current
            if(!furi_string_empty(app->meta->prev_0_path)) {
                path_extract_filename(app->meta->prev_0_path, temp_str, true);
                int w = canvas_string_width(canvas, furi_string_get_cstr(temp_str));
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_rbox(canvas, 1, 1, w + 4, 12, 2);
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_str_aligned(
                    canvas, 3, 3, AlignLeft, AlignTop, furi_string_get_cstr(temp_str));
            }

            // last 3
            canvas_set_color(canvas, ColorBlack);

            if(!furi_string_empty(app->meta->prev_1_path)) {
                path_extract_filename(app->meta->prev_1_path, temp_str, true);
                canvas_draw_str_aligned(
                    canvas, 3, 15, AlignLeft, AlignTop, furi_string_get_cstr(temp_str));
            }

            if(!furi_string_empty(app->meta->prev_2_path)) {
                path_extract_filename(app->meta->prev_2_path, temp_str, true);
                canvas_draw_str_aligned(
                    canvas, 3, 26, AlignLeft, AlignTop, furi_string_get_cstr(temp_str));
            }

            if(!furi_string_empty(app->meta->prev_3_path)) {
                path_extract_filename(app->meta->prev_3_path, temp_str, true);
                canvas_draw_str_aligned(
                    canvas, 3, 37, AlignLeft, AlignTop, furi_string_get_cstr(temp_str));
            }
        }
        break;
    default:
        break;
    }

    furi_string_free(temp_str);
    furi_mutex_release(app->mutex);
}

static void input_callback(InputEvent* event, void* ctx) {
    Playlist* app = ctx;
    furi_message_queue_put(app->input_queue, event, 0);
}

////////////////////////////////////////////////////////////////////////////////

Playlist* playlist_alloc(DisplayMeta* meta) {
    Playlist* app = malloc(sizeof(Playlist));
    app->file_path = furi_string_alloc();
    furi_string_set(app->file_path, PLAYLIST_FOLDER);

    app->meta = meta;
    app->worker = NULL;

    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->input_queue = furi_message_queue_alloc(32, sizeof(InputEvent));

    // view port
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, render_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);

    // gui
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    return app;
}

void playlist_start_worker(Playlist* app, DisplayMeta* meta) {
    app->worker = playlist_worker_alloc(meta);

    // count playlist items
    Storage* storage = furi_record_open(RECORD_STORAGE);
    app->meta->total_count =
        playlist_count_playlist_items(storage, furi_string_get_cstr(app->file_path));
    furi_record_close(RECORD_STORAGE);

    // start thread
    playlist_worker_start(app->worker, furi_string_get_cstr(app->file_path));
}

void playlist_free(Playlist* app) {
    furi_string_free(app->file_path);

    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(app->view_port);

    furi_message_queue_free(app->input_queue);
    furi_mutex_free(app->mutex);

    playlist_meta_free(app->meta);

    free(app);
}

int32_t playlist_app(void* p) {
    UNUSED(p);

    // create playlist folder
    {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        if(!storage_simply_mkdir(storage, PLAYLIST_FOLDER)) {
            FURI_LOG_E(TAG, "Could not create folder %s", PLAYLIST_FOLDER);
        }
        furi_record_close(RECORD_STORAGE);
    }

    // create app
    DisplayMeta* meta = playlist_meta_alloc();
    Playlist* app = playlist_alloc(meta);
    meta->view_port = app->view_port;

    // Enable power for External CC1101 if it is connected
    furi_hal_subghz_enable_ext_power();
    // Auto switch to internal radio if external radio is not available
    furi_delay_ms(15);
    if(!furi_hal_subghz_check_radio()) {
        furi_hal_subghz_select_radio_type(SubGhzRadioInternal);
        furi_hal_subghz_init_radio_type(SubGhzRadioInternal);
    }

    furi_hal_power_suppress_charge_enter();

    // select playlist file
    {
        DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
        DialogsFileBrowserOptions browser_options;
        dialog_file_browser_set_basic_options(&browser_options, PLAYLIST_EXT, &I_sub1_10px);
        browser_options.hide_ext = false;

        const bool res =
            dialog_file_browser_show(dialogs, app->file_path, app->file_path, &browser_options);
        furi_record_close(RECORD_DIALOGS);
        // check if a file was selected
        if(!res) {
            FURI_LOG_E(TAG, "No file selected");
            goto exit_cleanup;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

    playlist_start_worker(app, meta);
    meta_set_state(app->meta, STATE_OVERVIEW);

    bool exit_loop = false;
    InputEvent input;
    while(1) { // close application if no file was selected
        furi_check(
            furi_message_queue_get(app->input_queue, &input, FuriWaitForever) == FuriStatusOk);

        switch(input.key) {
        case InputKeyLeft:
            if(app->meta->state == STATE_OVERVIEW) {
                if(input.type == InputTypeShort && app->meta->playlist_repetitions > 0) {
                    --app->meta->playlist_repetitions;
                }
            } else if(app->meta->state == STATE_SENDING) {
                if(input.type == InputTypeShort) {
                    app->worker->ctl_request_prev = true;
                }
            }
            break;

        case InputKeyRight:
            if(app->meta->state == STATE_OVERVIEW) {
                if(input.type == InputTypeShort) {
                    ++app->meta->playlist_repetitions;
                }
            } else if(app->meta->state == STATE_SENDING) {
                if(input.type == InputTypeShort) {
                    app->worker->ctl_request_skip = true;
                }
            }
            break;

        case InputKeyOk:
            if(input.type == InputTypeShort) {
                // toggle pause state
                if(!app->worker->is_running) {
                    app->worker->ctl_pause = false;
                    app->worker->ctl_request_exit = false;
                    playlist_worker_start(app->worker, furi_string_get_cstr(app->file_path));
                } else {
                    app->worker->ctl_pause = !app->worker->ctl_pause;
                }
            }
            break;
        case InputKeyBack:
            FURI_LOG_D(TAG, "Pressed Back button. Application will exit");
            exit_loop = true;
            break;
        default:
            break;
        }

        furi_mutex_release(app->mutex);

        // exit application
        if(exit_loop == true) {
            break;
        }

        view_port_update(app->view_port);
    }

exit_cleanup:

    furi_hal_power_suppress_charge_exit();
    // Disable power for External CC1101 if it was enabled and module is connected
    furi_hal_subghz_disable_ext_power();
    // Reinit SPI handles for internal radio / nfc
    furi_hal_subghz_init_radio_type(SubGhzRadioInternal);

    if(app->worker != NULL) {
        if(playlist_worker_running(app->worker)) {
            FURI_LOG_D(TAG, "Thread is still running. Requesting thread to finish ...");
            playlist_worker_stop(app->worker);
        }
        FURI_LOG_D(TAG, "Freeing Worker ...");
        playlist_worker_free(app->worker);
    }

    FURI_LOG_D(TAG, "Freeing Playlist ...");
    playlist_free(app);
    return 0;
}
