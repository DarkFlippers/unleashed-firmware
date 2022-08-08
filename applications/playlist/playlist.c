#include <furi.h>

#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

#include <assets_icons.h>

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/path.h>
#include <applications/subghz/subghz_i.h>

#define PLAYLIST_FOLDER "/ext/playlist"
#define PLAYLIST_EXT ".txt"
#define TAG "Playlist"

#define STATE_OVERVIEW 2

#define WIDTH 128
#define HEIGHT 64

typedef struct {
    FuriThread* thread;
    Storage* storage;
    FlipperFormat* format;

    string_t file_path; // Path to the playlist file
    string_t current_file; // Path to the current file
    bool running; // True if the worker is running
} PlaylistWorker;

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    Gui* gui;

    PlaylistWorker* worker;

    string_t file_path; // Path to the playlist file

    int state; // Current state for rendering
} Playlist;

////////////////////////////////////////////////////////////////////////////////

static int32_t playlist_worker_thread(void* ctx) {
    PlaylistWorker* worker = ctx;
    FURI_LOG_I(TAG, "(worker) Worker start");

    if(!flipper_format_file_open_existing(worker->format, string_get_cstr(worker->file_path))) {
        FURI_LOG_E(TAG, "(worker) Could not open file %s", string_get_cstr(worker->file_path));
        worker->running = false;
        return 0;
    }

    FURI_LOG_I(TAG, "(worker) Opened file %s", string_get_cstr(worker->file_path));

    string_t data;
    string_init(data);

    int count = 0;

    Stream* stream = flipper_format_get_raw_stream(worker->format);
    while(worker->running && stream_read_line(stream, data)) {
        string_strim(data);
        FURI_LOG_I(TAG, "(worker) Read line %s", string_get_cstr(data));

        char* str;
        str = strstr(string_get_cstr(data), "SUB: ");
        if(str != NULL) {
            str = strchr(str, ' ');

            while(strchr(str, ' ') != NULL) {
                str = strchr(str, ' ');
                str += 1;

                count++;
                FURI_LOG_I(TAG, "(worker)  data %d: %s", count, str);

                // show current file
                string_set_str(worker->current_file, str);
                FURI_LOG_I(TAG, "(worker)  current_file: %s", worker->current_file);

                furi_delay_ms(3000); // TODO: remove this delay
            }
        }
    }
    flipper_format_file_close(worker->format);

    string_clear(data);
    FURI_LOG_I(TAG, "Done reading. Read %d data lines.", count);

    worker->running = false;

    return 0;
}

PlaylistWorker* playlist_worker_alloc() {
    PlaylistWorker* instance = malloc(sizeof(PlaylistWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "PlaylistWorker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, playlist_worker_thread);

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->format = flipper_format_file_alloc(instance->storage);

    string_init(instance->file_path);
    string_init(instance->current_file);

    return instance;
}

void playlist_worker_free(PlaylistWorker* instance) {
    furi_assert(instance);

    furi_thread_free(instance->thread);
    flipper_format_free(instance->format);
    furi_record_close(RECORD_STORAGE);

    string_clear(instance->file_path);
    string_clear(instance->current_file);

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

    furi_thread_start(instance->thread);
}

////////////////////////////////////////////////////////////////////////////////

static void render_callback(Canvas* canvas, void* ctx) {
    Playlist* app = ctx;
    furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);

    canvas_clear(canvas);

    switch(app->state) {
    case STATE_OVERVIEW:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 5, HEIGHT - 5, AlignLeft, AlignBottom, "FILE:");

        // extract file name from file_path
        {
            int padL = canvas_string_width(canvas, "FILE: ");

            string_t file_name;
            string_init(file_name);
            path_extract_filename(app->file_path, file_name, true);
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(
                canvas, 5 + padL, HEIGHT - 5, AlignLeft, AlignBottom, string_get_cstr(file_name));
            string_clear(file_name);
        }

        if(app->worker != NULL && app->worker->running == true &&
           !string_empty_p(app->worker->current_file)) {
            string_t file_name;
            string_init(file_name);
            path_extract_filename(app->worker->current_file, file_name, true);
            canvas_draw_str_aligned(canvas, 5, 5, AlignLeft, AlignTop, string_get_cstr(file_name));
            FURI_LOG_I(TAG, "(render) drawing current file %s", string_get_cstr(file_name));
            string_clear(file_name);
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

Playlist* playlist_alloc() {
    Playlist* app = malloc(sizeof(Playlist));
    app->state = 0;
    string_init(app->file_path);

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

    free(app);
}

int32_t playlist_app(void* p) {
    UNUSED(p);

    // create app
    Playlist* app = playlist_alloc();

    // create playlist folder
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_simply_mkdir(storage, PLAYLIST_FOLDER)) {
        FURI_LOG_E(TAG, "Could not create folder %s", PLAYLIST_FOLDER);
    }
    furi_record_close(RECORD_STORAGE);

    string_set_str(app->file_path, PLAYLIST_FOLDER);

    // select playlist file
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    const bool res = dialog_file_browser_show(
        dialogs, app->file_path, app->file_path, PLAYLIST_EXT, true, &I_sub1_10px, true);
    furi_record_close(RECORD_DIALOGS);

    do {
        // check if a file was selected
        if(!res) {
            FURI_LOG_E(TAG, "No file selected");
            break;
        }

        app->state = STATE_OVERVIEW;

        FURI_LOG_I(TAG, "Starting thread ...");
        app->worker = playlist_worker_alloc();
        playlist_worker_start(app->worker, string_get_cstr(app->file_path));

        bool exit_loop = false;
        InputEvent input;
        while(res) { // close application if no file was selected
            FURI_LOG_I(TAG, "Checking queue");
            furi_check(
                furi_message_queue_get(app->input_queue, &input, FuriWaitForever) == FuriStatusOk);

            FURI_LOG_I(
                TAG,
                "Key: %s, Type: %s",
                input_get_key_name(input.key),
                input_get_type_name(input.type));

            switch(input.key) {
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
    } while(0);

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