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

#define BRUTE_HELPER_FOLDER "/ext/brutehelper"
#define BRUTE_HELPER_EXT ".txt"
#define TAG "Brute Helper"

#define WIDTH 128
#define HEIGHT 64

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    Gui* gui;

    string_t file_path;
    /* data */
} BruteHelper;

static void render_callback(Canvas* canvas, void* ctx) {
    BruteHelper* app = ctx;
    furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, WIDTH / 2, HEIGHT / 2, AlignCenter, AlignTop, "Hello World!");

    furi_mutex_release(app->mutex);
}

static void input_callback(InputEvent* event, void* ctx) {
    BruteHelper* app = ctx;
    furi_message_queue_put(app->input_queue, event, 0);
}

BruteHelper* brute_helper_alloc() {
    BruteHelper* app = malloc(sizeof(BruteHelper));
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

void brute_helper_free(BruteHelper* app) {
    string_clear(app->file_path);

    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(app->view_port);

    furi_message_queue_free(app->input_queue);
    furi_mutex_free(app->mutex);

    free(app);
}

int32_t brute_helper_app(void* p) {
    UNUSED(p);

    // create app
    BruteHelper* app = brute_helper_alloc();

    // init app
    string_init(app->file_path);

    // create brute_helper folder
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_simply_mkdir(storage, BRUTE_HELPER_FOLDER)) {
        FURI_LOG_E(TAG, "Could not create folder %s", BRUTE_HELPER_FOLDER);
    }
    furi_record_close(RECORD_STORAGE);

    string_set_str(app->file_path, BRUTE_HELPER_FOLDER);

    // select brute file
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    const bool res = dialog_file_browser_show(
        dialogs, app->file_path, app->file_path, BRUTE_HELPER_EXT, true, &I_sub1_10px, true);
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

    brute_helper_free(app);
    return 0;
}