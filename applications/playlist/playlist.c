#include <furi.h>

#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

#include <assets_icons.h>

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/path.h>
#include <applications/subghz/subghz_i.h>

#include "flipper_format_stream.h"
#include "flipper_format_stream_i.h"

#include "playlist_file.h"

#define PLAYLIST_FOLDER "/ext/playlist"
#define PLAYLIST_EXT ".txt"
#define TAG "Playlist"

#define STATE_OVERVIEW 2

#define WIDTH 128
#define HEIGHT 64

typedef struct {
    int current_count; // Number of processed files
    int total_count; // Number of items in the playlist

    // last 3 files
    string_t prev_0_path; // current file
    string_t prev_1_path; // previous file
    string_t prev_2_path;
    string_t prev_3_path;
} DisplayMeta;

typedef struct {
    FuriThread* thread;
    Storage* storage;
    FlipperFormat* format;

    DisplayMeta* meta;

    string_t file_path; // Path to the playlist file
    bool running; // indicates if the worker is running
    bool paused; // can be set to true to pause worker
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
        worker->running = false;
        return 0;
    }

    // reset worker meta

    string_t data;
    string_init(data);
    while(worker->running && flipper_format_read_string(worker->format, "sub", data)) {
        // wait if paused
        while(worker->paused) {
            furi_delay_ms(100);
        }

        // send .sub files
        ++worker->meta->current_count;
        const char* str = string_get_cstr(data);
        FURI_LOG_I(TAG, "(worker)  data #%d: %s", worker->meta->current_count, str);

        // it's not fancy, but it works :)
        string_reset(worker->meta->prev_3_path);
        string_set_str(worker->meta->prev_3_path, string_get_cstr(worker->meta->prev_2_path));
        string_reset(worker->meta->prev_2_path);
        string_set_str(worker->meta->prev_2_path, string_get_cstr(worker->meta->prev_1_path));
        string_reset(worker->meta->prev_1_path);
        string_set_str(worker->meta->prev_1_path, string_get_cstr(worker->meta->prev_0_path));
        string_reset(worker->meta->prev_0_path);
        string_set_str(worker->meta->prev_0_path, str);

        FURI_LOG_I(TAG, "");
        FURI_LOG_I(TAG, "(worker)  prev_3: %s", string_get_cstr(worker->meta->prev_3_path));
        FURI_LOG_I(TAG, "(worker)  prev_2: %s", string_get_cstr(worker->meta->prev_2_path));
        FURI_LOG_I(TAG, "(worker)  prev_1: %s", string_get_cstr(worker->meta->prev_1_path));
        FURI_LOG_I(TAG, "(worker)  prev_0: %s", string_get_cstr(worker->meta->prev_0_path));
        FURI_LOG_I(TAG, "");

        furi_delay_ms(1500); // TODO: remove this delay
    }
    flipper_format_file_close(worker->format);
    string_clear(data);

    FURI_LOG_I(TAG, "Done reading. Read %d data lines.", worker->meta->current_count);
    worker->running = false;
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

    instance->paused = true; // require the user to manually start the worker

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
    furi_assert(worker->running);

    worker->running = false;
    furi_thread_join(worker->thread);
}

bool playlist_worker_running(PlaylistWorker* worker) {
    furi_assert(worker);
    return worker->running;
}

void playlist_worker_start(PlaylistWorker* instance, const char* file_path) {
    furi_assert(instance);
    furi_assert(!instance->running);

    string_set_str(instance->file_path, file_path);
    instance->running = true;

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

        // draw controls
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_set_color(canvas, ColorBlack);
            if(!app->worker->running) {
                canvas_draw_str_aligned(canvas, 5, 5, AlignLeft, AlignTop, "[OK]: Start");
            } else if(app->worker->paused) {
                canvas_draw_str_aligned(canvas, 5, 5, AlignLeft, AlignTop, "[OK]: Resume");
            } else {
                canvas_draw_str_aligned(canvas, 5, 5, AlignLeft, AlignTop, "[OK]: Pause");
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

    FURI_LOG_I(TAG, "Starting thread ...");
    app->worker = playlist_worker_alloc(meta);

    // count playlist items
    {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        app->meta->total_count =
            playlist_count_playlist_items(storage, string_get_cstr(app->file_path));
        FURI_LOG_I(TAG, "Selected file contains %d playlist items.", app->meta->total_count);
        furi_record_close(RECORD_STORAGE);
    }

    // start thread
    playlist_worker_start(app->worker, string_get_cstr(app->file_path));

    app->state = STATE_OVERVIEW;

    bool exit_loop = false;
    InputEvent input;
    while(1) { // close application if no file was selected
        FURI_LOG_I(TAG, "Checking queue");
        furi_check(
            furi_message_queue_get(app->input_queue, &input, FuriWaitForever) == FuriStatusOk);

        FURI_LOG_I(
            TAG,
            "Key: %s, Type: %s",
            input_get_key_name(input.key),
            input_get_type_name(input.type));

        switch(input.key) {
        case InputKeyOk:
            // toggle pause state
            if(!app->worker->running) {
                FURI_LOG_I(TAG, "Worker is NOT running. Starting worker.");
                playlist_worker_start(app->worker, string_get_cstr(app->file_path));
            } else {
                FURI_LOG_I(TAG, "Worker IS running. Toggled pause state.");
                app->worker->paused = !app->worker->paused;
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