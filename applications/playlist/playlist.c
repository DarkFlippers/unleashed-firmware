#include <furi.h>

#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>

#include <assets_icons.h>

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/path.h>
#include <applications/subghz/subghz_i.h>

#define PLAYLIST_FOLDER "/ext/playlist"
#define PLAYLIST_EXT ".txt"
#define TAG "Playlist"

#define WIDTH 128
#define HEIGHT 64

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    Gui* gui;

    string_t file_path;
} Playlist;

static void render_callback(Canvas* canvas, void* ctx) {
    Playlist* app = ctx;
    furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, WIDTH / 2, HEIGHT / 2 - 5, AlignCenter, AlignTop, "Hello World");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas, WIDTH / 2, HEIGHT / 2 + 10, AlignCenter, AlignTop, "from Playlist");

    furi_mutex_release(app->mutex);
}

static void input_callback(InputEvent* event, void* ctx) {
    Playlist* app = ctx;
    furi_message_queue_put(app->input_queue, event, 0);
}

Playlist* playlist_alloc() {
    Playlist* app = malloc(sizeof(Playlist));
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

    // init app
    string_init(app->file_path);

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
    if(!res) {
        FURI_LOG_E(TAG, "No file selected");
    }

    bool exit_loop = false;
    InputEvent input;
    while(1) {
        furi_check(
            furi_message_queue_get(app->input_queue, &input, FuriWaitForever) == FuriStatusOk);
        FURI_LOG_I(
            TAG,
            "Key: %s, Type: %s",
            input_get_key_name(input.key),
            input_get_type_name(input.type));

        switch(input.key) {
        case InputKeyBack:
            exit_loop = true;
            break;

        default:
            break;
        }

        furi_mutex_release(app->mutex);

        if(exit_loop == true) {
            break;
        }

        view_port_update(app->view_port);
    }

    playlist_free(app);
    return 0;
}