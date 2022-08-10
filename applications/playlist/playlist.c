#include <furi.h>

#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

#include <lib/toolbox/path.h>
#include <assets_icons.h>

#include <flipper_format/flipper_format_i.h>
#include <applications/subghz/subghz_i.h>

#include "flipper_format_stream.h"
#include "flipper_format_stream_i.h"

#include <lib/subghz/transmitter.h>
#include <lib/subghz/protocols/raw.h>

#include "playlist_file.h"

#define PLAYLIST_FOLDER "/ext/playlist"
#define PLAYLIST_EXT ".txt"
#define TAG "Playlist"

#define STATE_OVERVIEW 2

#define WIDTH 128
#define HEIGHT 64

typedef struct {
    int current_count; // number of processed files
    int total_count; // number of items in the playlist

    // last 3 files
    string_t prev_0_path; // current file
    string_t prev_1_path; // previous file
    string_t prev_2_path; // previous previous file
    string_t prev_3_path; // you get the idea
} DisplayMeta;

typedef struct {
    FuriThread* thread;
    Storage* storage;
    FlipperFormat* format;

    DisplayMeta* meta;

    string_t file_path; // path to the playlist file

    bool ctl_request_exit; // can be set to true if the worker should exit
    bool ctl_pause; // can be set to true if the worker should pause

    bool is_running; // indicates if the worker is running
} PlaylistWorker;

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    Gui* gui;

    DisplayMeta* meta;
    PlaylistWorker* worker;

    string_t file_path; // Path to the playlist file

    int state; // Current state for rendering
} Playlist;

////////////////////////////////////////////////////////////////////////////////

static int32_t playlist_worker_thread(void* ctx) {
    PlaylistWorker* worker = ctx;
    if(!flipper_format_file_open_existing(worker->format, string_get_cstr(worker->file_path))) {
        worker->is_running = false;
        return 0;
    }

    // allocate subghz environment
    SubGhzEnvironment* environment = subghz_environment_alloc();

    string_t data, preset, protocol;
    string_init(data);
    string_init(preset);
    string_init(protocol);

    FlipperFormat* fff_data = flipper_format_string_alloc();

    while(flipper_format_read_string(worker->format, "sub", data)) {
        // wait if paused
        while(!worker->ctl_request_exit && worker->ctl_pause) {
            FURI_LOG_I(TAG, "Just Paused. Waiting...");
            furi_delay_ms(100);
        }
        // exit loop if requested to stop
        if(worker->ctl_request_exit) {
            FURI_LOG_I(TAG, "Requested to exit. Exiting loop...");
            break;
        }

        // send .sub files
        ++worker->meta->current_count;
        const char* str = string_get_cstr(data);

        FURI_LOG_I(TAG, "(worker) Sending %s", str);

        // it's not fancy, but it works for now :)
        string_reset(worker->meta->prev_3_path);
        string_set_str(worker->meta->prev_3_path, string_get_cstr(worker->meta->prev_2_path));
        string_reset(worker->meta->prev_2_path);
        string_set_str(worker->meta->prev_2_path, string_get_cstr(worker->meta->prev_1_path));
        string_reset(worker->meta->prev_1_path);
        string_set_str(worker->meta->prev_1_path, string_get_cstr(worker->meta->prev_0_path));
        string_reset(worker->meta->prev_0_path);
        string_set_str(worker->meta->prev_0_path, str);

        // actual sending of .sub file
        {
            // open .sub file
            FlipperFormat* fff_file = flipper_format_file_alloc(worker->storage);

            if(!flipper_format_file_open_existing(fff_file, str)) {
                FURI_LOG_E(TAG, "  (TX) Failed to open %s", str);
                flipper_format_free(fff_file);
                continue;
            }

            // read frequency or default to 433.92MHz
            uint32_t frequency = 0;
            if(!flipper_format_read_uint32(fff_file, "Frequency", &frequency, 1)) {
                FURI_LOG_W(TAG, "  (TX) Missing Frequency, defaulting to 433.92MHz");
                frequency = 433920000;
            }
            FURI_LOG_I(TAG, "  (TX) Frequency: %u", frequency);
            // TODO: check if freq is allowed to transmit

            // check if preset is present
            if(!flipper_format_read_string(fff_file, "Preset", preset)) {
                FURI_LOG_E(TAG, "  (TX) Missing Preset");
                flipper_format_free(fff_file);
                continue;
            }

            FuriHalSubGhzPreset enum_preset;
            if(string_cmp_str(preset, "FuriHalSubGhzPresetOok270Async") == 0) {
                enum_preset = FuriHalSubGhzPresetOok270Async;
                FURI_LOG_I(TAG, "  (TX) Preset: Ook270Async");
            } else if(string_cmp_str(preset, "FuriHalSubGhzPresetOok650Async") == 0) {
                enum_preset = FuriHalSubGhzPresetOok650Async;
                FURI_LOG_I(TAG, "  (TX) Preset: Ook650Async");
            } else if(string_cmp_str(preset, "FuriHalSubGhzPreset2FSKDev238Async") == 0) {
                enum_preset = FuriHalSubGhzPreset2FSKDev238Async;
                FURI_LOG_I(TAG, "  (TX) Preset: 2FSKDev238Async");
            } else if(string_cmp_str(preset, "FuriHalSubGhzPreset2FSKDev476Async") == 0) {
                enum_preset = FuriHalSubGhzPreset2FSKDev476Async;
                FURI_LOG_I(TAG, "  (TX) Preset: 2FSKDev476Async");
            } else if(string_cmp_str(preset, "FuriHalSubGhzPresetMSK99_97KbAsync") == 0) {
                enum_preset = FuriHalSubGhzPresetMSK99_97KbAsync;
                FURI_LOG_I(TAG, "  (TX) Preset: MSK99_97KbAsync");
            } else if(string_cmp_str(preset, "FuriHalSubGhzPresetMSK99_97KbAsync") == 0) {
                enum_preset = FuriHalSubGhzPresetMSK99_97KbAsync;
                FURI_LOG_I(TAG, "  (TX) Preset: MSK99_97KbAsync");
            } else if(string_cmp_str(preset, "FuriHalSubGhzPresetCustom") == 0) {
                enum_preset = FuriHalSubGhzPresetCustom;
                FURI_LOG_I(TAG, "  (TX) Preset: Custom");
            } else {
                FURI_LOG_E(TAG, "  (TX) Invalid Preset");
                flipper_format_free(fff_file);
                continue;
            }

            // check if protocol is present
            if(!flipper_format_read_string(fff_file, "Protocol", protocol)) {
                FURI_LOG_E(TAG, "  (TX) Missing Protocol");
                flipper_format_free(fff_file);
                continue;
            }
            FURI_LOG_I(TAG, "  (TX) Protocol: %s", string_get_cstr(protocol));

            if(!string_cmp_str(protocol, "RAW")) {
                subghz_protocol_raw_gen_fff_data(fff_data, str);
            } else {
                stream_copy_full(
                    flipper_format_get_raw_stream(fff_file),
                    flipper_format_get_raw_stream(fff_data));
            }
            flipper_format_free(fff_file);

            // (try to) send file
            SubGhzTransmitter* transmitter =
                subghz_transmitter_alloc_init(environment, string_get_cstr(protocol));

            subghz_transmitter_deserialize(transmitter, fff_data);

            furi_hal_subghz_reset();
            furi_hal_subghz_load_preset(enum_preset);

            frequency = furi_hal_subghz_set_frequency_and_path(frequency);

            furi_hal_power_suppress_charge_enter();

            FURI_LOG_I(TAG, "  (TX) Start sending ...");

            furi_hal_subghz_start_async_tx(subghz_transmitter_yield, transmitter);
            while(!furi_hal_subghz_is_async_tx_complete()) {
                if(worker->ctl_request_exit || worker->ctl_pause) {
                    FURI_LOG_I(TAG, "    (TX) Requested to exit. Cancelling sending...");
                    break;
                }
                FURI_LOG_I(TAG, "    (TX) Sending...");
                furi_delay_ms(100);
            }
            FURI_LOG_I(TAG, "  (TX) Done sending.");
            furi_hal_subghz_stop_async_tx();
            furi_hal_subghz_sleep();

            furi_hal_power_suppress_charge_exit();

            subghz_transmitter_free(transmitter);
        } // end of start_send section
    } // end of loop

    FURI_LOG_I(TAG, "Exited Loop. Clean Up.");
    string_clear(data);
    string_clear(preset);
    string_clear(protocol);

    FURI_LOG_I(TAG, "  Cleaning up TX");
    subghz_environment_free(environment);

    flipper_format_file_close(worker->format);

    FURI_LOG_I(TAG, "Done reading. Read %d data lines.", worker->meta->current_count);
    worker->is_running = false;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

void playlist_meta_reset(DisplayMeta* instance) {
    instance->current_count = 0;
    string_clear(instance->prev_0_path);
    string_clear(instance->prev_1_path);
    string_clear(instance->prev_2_path);
    string_clear(instance->prev_3_path);
}

DisplayMeta* playlist_meta_alloc() {
    DisplayMeta* instance = malloc(sizeof(DisplayMeta));
    string_init(instance->prev_0_path);
    string_init(instance->prev_1_path);
    string_init(instance->prev_2_path);
    string_init(instance->prev_3_path);
    playlist_meta_reset(instance);
    return instance;
}

void playlist_meta_free(DisplayMeta* instance) {
    string_clear(instance->prev_0_path);
    string_clear(instance->prev_1_path);
    string_clear(instance->prev_2_path);
    string_clear(instance->prev_3_path);
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

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->format = flipper_format_file_alloc(instance->storage);
    instance->meta = meta;

    instance->ctl_pause = true; // require the user to manually start the worker

    string_init(instance->file_path);

    return instance;
}

void playlist_worker_free(PlaylistWorker* instance) {
    furi_assert(instance);

    furi_thread_free(instance->thread);
    flipper_format_free(instance->format);
    furi_record_close(RECORD_STORAGE);

    string_clear(instance->file_path);

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

    string_set_str(instance->file_path, file_path);
    instance->is_running = true;

    // reset meta (current/total)
    playlist_meta_reset(instance->meta);

    furi_thread_start(instance->thread);
}

////////////////////////////////////////////////////////////////////////////////

static void render_callback(Canvas* canvas, void* ctx) {
    Playlist* app = ctx;
    furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);

    canvas_clear(canvas);

    switch(app->state) {
    case STATE_OVERVIEW:
        // draw progress bar
        {
            double progress = (double)app->meta->current_count / (double)app->meta->total_count;
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_rframe(canvas, 1, HEIGHT - 12, WIDTH - 2, 11, 2);

            if(progress > 0) {
                int progress_width = (int)(progress * (double)(WIDTH - 2));
                canvas_draw_rbox(canvas, 1, HEIGHT - 12, progress_width, 11, 2);
            }

            // draw progress text
            string_t progress_text;
            string_init(progress_text);
            string_printf(
                progress_text, "%d/%d", app->meta->current_count, app->meta->total_count);

            if(progress >= (double).5) {
                canvas_set_color(canvas, ColorWhite);
            } else {
                canvas_set_color(canvas, ColorBlack);
            }
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(
                canvas,
                WIDTH / 2,
                HEIGHT - 3,
                AlignCenter,
                AlignBottom,
                string_get_cstr(progress_text));

            string_clear(progress_text);
        }

        // draw last and current file
        {
            canvas_set_color(canvas, ColorBlack);

            string_t path;
            string_init(path);

            canvas_set_font(canvas, FontSecondary);

            // current
            if(!string_empty_p(app->meta->prev_0_path)) {
                path_extract_filename(app->meta->prev_0_path, path, true);
                int w = canvas_string_width(canvas, string_get_cstr(path));
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_rbox(canvas, 1, 1, w + 4, 12, 2);
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_str_aligned(canvas, 3, 3, AlignLeft, AlignTop, string_get_cstr(path));
                string_reset(path);
            }

            // last 3
            canvas_set_color(canvas, ColorBlack);

            if(!string_empty_p(app->meta->prev_1_path)) {
                path_extract_filename(app->meta->prev_1_path, path, true);
                canvas_draw_str_aligned(canvas, 3, 15, AlignLeft, AlignTop, string_get_cstr(path));
                string_reset(path);
            }

            if(!string_empty_p(app->meta->prev_2_path)) {
                path_extract_filename(app->meta->prev_2_path, path, true);
                canvas_draw_str_aligned(canvas, 6, 26, AlignLeft, AlignTop, string_get_cstr(path));
                string_reset(path);
            }

            if(!string_empty_p(app->meta->prev_3_path)) {
                path_extract_filename(app->meta->prev_3_path, path, true);
                canvas_draw_str_aligned(canvas, 9, 37, AlignLeft, AlignTop, string_get_cstr(path));
                string_reset(path);
            }

            string_clear(path);
        }

        // draw controls
        {
            canvas_set_font(canvas, FontSecondary);

            const int ctl_w = 24;
            const int ctl_h = 18;

            // draw background
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_rbox(canvas, WIDTH - ctl_w, HEIGHT / 2 - ctl_h / 2, ctl_w, ctl_h, 3);
            canvas_draw_box(canvas, WIDTH - 3, HEIGHT / 2 - ctl_h / 2, 3, ctl_h); // right corner

            // draw circle (OK)
            canvas_set_color(canvas, ColorWhite);

            const int disc_r = 3;
            canvas_draw_disc(
                canvas, WIDTH - ctl_w / 2, HEIGHT / 2 - ctl_h / 2 + disc_r + 1, disc_r);

            // draw texts
            if(!app->worker->is_running) {
                canvas_draw_str_aligned(
                    canvas, WIDTH - ctl_w / 2, HEIGHT / 2 + 4, AlignCenter, AlignCenter, "STA");
            } else if(app->worker->ctl_pause) {
                canvas_draw_str_aligned(
                    canvas, WIDTH - ctl_w / 2, HEIGHT / 2 + 4, AlignCenter, AlignCenter, "RES");
            } else {
                canvas_draw_str_aligned(
                    canvas, WIDTH - ctl_w / 2, HEIGHT / 2 + 4, AlignCenter, AlignCenter, "PAU");
            }
        }
        break;
    }

    furi_mutex_release(app->mutex);
}

static void input_callback(InputEvent* event, void* ctx) {
    Playlist* app = ctx;
    furi_message_queue_put(app->input_queue, event, 0);
}

////////////////////////////////////////////////////////////////////////////////

Playlist* playlist_alloc(DisplayMeta* meta) {
    Playlist* app = malloc(sizeof(Playlist));
    app->state = 0;

    string_init(app->file_path);
    string_set_str(app->file_path, PLAYLIST_FOLDER);

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

void playlist_free(Playlist* app) {
    string_clear(app->file_path);

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

    // select playlist file
    {
        DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
        const bool res = dialog_file_browser_show(
            dialogs, app->file_path, app->file_path, PLAYLIST_EXT, true, &I_sub1_10px, true);
        furi_record_close(RECORD_DIALOGS);
        // check if a file was selected
        if(!res) {
            FURI_LOG_E(TAG, "No file selected");
            goto exit_cleanup;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

    app->worker = playlist_worker_alloc(meta);

    // count playlist items
    {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        app->meta->total_count =
            playlist_count_playlist_items(storage, string_get_cstr(app->file_path));
        furi_record_close(RECORD_STORAGE);
    }

    // start thread
    playlist_worker_start(app->worker, string_get_cstr(app->file_path));

    app->state = STATE_OVERVIEW;

    bool exit_loop = false;
    InputEvent input;
    while(1) { // close application if no file was selected
        furi_check(
            furi_message_queue_get(app->input_queue, &input, FuriWaitForever) == FuriStatusOk);

        switch(input.key) {
        case InputKeyOk:
            // toggle pause state
            if(!app->worker->is_running) {
                playlist_worker_start(app->worker, string_get_cstr(app->file_path));
            } else {
                app->worker->ctl_pause = !app->worker->ctl_pause;
            }
            break;
        case InputKeyBack:
            FURI_LOG_I(TAG, "Pressed Back button. Application will exit");
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
    if(app->worker != NULL) {
        if(playlist_worker_running(app->worker)) {
            FURI_LOG_I(TAG, "Thread is still running. Requesting thread to finish ...");
            playlist_worker_stop(app->worker);
        }
        playlist_worker_free(app->worker);
    }

    playlist_free(app);
    return 0;
}
