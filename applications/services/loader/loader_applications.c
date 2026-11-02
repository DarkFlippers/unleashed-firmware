#include "loader.h"
#include "loader_applications.h"
#include <flipper_application/flipper_application.h>
#include <assets_icons.h>
#include <gui/gui.h>
#include <gui/view_holder.h>
#include <gui/modules/file_browser.h>
#include <dolphin/dolphin.h>
#include <lib/toolbox/path.h>

#define TAG "LoaderApplications"

#define APPS_PATH       EXT_PATH("apps")
#define JS_RUNNER_APP   APPS_PATH "/assets/js_app.fap"
#define PATH_QUEUE_SIZE 4

struct LoaderApplications {
    FuriThread* thread;
    void (*closed_cb)(void*);
    void* context;
};

static int32_t loader_applications_thread(void* p);

LoaderApplications* loader_applications_alloc(void (*closed_cb)(void*), void* context) {
    LoaderApplications* loader_applications = malloc(sizeof(LoaderApplications));
    loader_applications->thread =
        furi_thread_alloc_ex(TAG, 768, loader_applications_thread, (void*)loader_applications);
    loader_applications->closed_cb = closed_cb;
    loader_applications->context = context;
    furi_thread_start(loader_applications->thread);
    return loader_applications;
}

void loader_applications_free(LoaderApplications* loader_applications) {
    furi_assert(loader_applications);
    furi_thread_join(loader_applications->thread);
    furi_thread_free(loader_applications->thread);
    free(loader_applications);
}

typedef struct {
    FuriString* selected_path;

    Storage* storage;
    Loader* loader;
    // Not a ViewDispatcher: its input callback blocks the GUI thread once its queue
    // fills (view_dispatcher.c:276), and starting an application parks this thread
    FuriMessageQueue* queue;

    ViewHolder* view_holder;
    FileBrowser* browser;
} LoaderApplicationsApp;

static bool loader_applications_item_callback(
    FuriString* path,
    void* context,
    uint8_t** icon_ptr,
    FuriString* item_name) {
    LoaderApplicationsApp* loader_applications_app = context;
    furi_assert(loader_applications_app);
    if(furi_string_end_with(path, ".fap")) {
        return flipper_application_load_name_and_icon(
            path, loader_applications_app->storage, icon_ptr, item_name);
    } else {
        path_extract_filename(path, item_name, false);
        memcpy(*icon_ptr, icon_get_frame_data(&I_js_script_10px, 0), FAP_MANIFEST_MAX_ICON_SIZE);
        return true;
    }
}

// A queued path is the application to start, NULL is Back on the root of the list.
// Runs on the GUI thread, so it must not wait for the queue: dropping a press costs
// the user another one, holding up the GUI thread costs everyone
static void loader_applications_queue_path(LoaderApplicationsApp* app, FuriString* path) {
    if(furi_message_queue_put(app->queue, &path, 0) != FuriStatusOk) {
        FURI_LOG_W(TAG, "Queue full, press dropped");
        if(path) furi_string_free(path);
    }
}

static void loader_applications_browser_callback(void* context) {
    LoaderApplicationsApp* app = context;
    // Copied on the thread that writes it: the next press rewrites selected_path,
    // and it can land while the application this one asks for is still starting
    loader_applications_queue_path(app, furi_string_alloc_set(app->selected_path));
}

static void loader_applications_back_callback(void* context) {
    LoaderApplicationsApp* app = context;
    loader_applications_queue_path(app, NULL);
}

static LoaderApplicationsApp* loader_applications_app_alloc(void) {
    LoaderApplicationsApp* app = malloc(sizeof(LoaderApplicationsApp)); //-V799
    app->selected_path = furi_string_alloc_set(APPS_PATH);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->loader = furi_record_open(RECORD_LOADER);
    app->queue = furi_message_queue_alloc(PATH_QUEUE_SIZE, sizeof(FuriString*));
    Gui* gui = furi_record_open(RECORD_GUI);

    app->browser = file_browser_alloc(app->selected_path);
    file_browser_configure(
        app->browser, ".fap|.js", APPS_PATH, true, false, &I_unknown_10px, true);
    file_browser_set_callback(app->browser, loader_applications_browser_callback, app);
    file_browser_set_item_callback(app->browser, loader_applications_item_callback, app);
    file_browser_start(app->browser, app->selected_path);

    app->view_holder = view_holder_alloc();
    view_holder_attach_to_gui(app->view_holder, gui);
    view_holder_set_back_callback(app->view_holder, loader_applications_back_callback, app);
    // Set once for the whole session, so the list is never rebuilt and is what the
    // GUI draws the moment an application takes its own view port down
    view_holder_set_view(app->view_holder, file_browser_get_view(app->browser));

    return app;
} //-V773

static void loader_applications_app_free(LoaderApplicationsApp* app) {
    furi_assert(app);

    view_holder_set_view(app->view_holder, NULL);
    file_browser_stop(app->browser);
    file_browser_free(app->browser);
    view_holder_free(app->view_holder);
    furi_record_close(RECORD_GUI);

    // A press still queued owns its path
    FuriString* path;
    while(furi_message_queue_get(app->queue, &path, 0) == FuriStatusOk) {
        if(path) furi_string_free(path);
    }
    furi_message_queue_free(app->queue);
    furi_record_close(RECORD_LOADER);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(app->selected_path);
    free(app);
}

#define APPLICATION_STOP_EVENT 1

static void loader_pubsub_callback(const void* message, void* context) {
    const LoaderEvent* event = message;
    const FuriThreadId thread_id = (FuriThreadId)context;

    if(event->type == LoaderEventTypeNoMoreAppsInQueue) {
        furi_thread_flags_set(thread_id, APPLICATION_STOP_EVENT);
    }
}

static void
    loader_applications_start_app(LoaderApplicationsApp* app, const char* name, const char* args) {
    dolphin_deed(DolphinDeedPluginStart);

    // load app
    FuriThreadId thread_id = furi_thread_get_current_id();
    FuriPubSubSubscription* subscription =
        furi_pubsub_subscribe(loader_get_pubsub(app->loader), loader_pubsub_callback, thread_id);

    LoaderStatus status = loader_start_with_gui_error(app->loader, name, args);

    if(status == LoaderStatusOk) {
        furi_thread_flags_wait(APPLICATION_STOP_EVENT, FuriFlagWaitAny, FuriWaitForever);
    }

    furi_pubsub_unsubscribe(loader_get_pubsub(app->loader), subscription);
    furi_thread_flags_clear(APPLICATION_STOP_EVENT);
}

static int32_t loader_applications_thread(void* p) {
    LoaderApplications* loader_applications = p;
    LoaderApplicationsApp* app = loader_applications_app_alloc();

    FuriString* path;
    while(true) {
        furi_check(furi_message_queue_get(app->queue, &path, FuriWaitForever) == FuriStatusOk);
        if(!path) break;

        if(furi_string_end_with(path, ".js")) {
            loader_applications_start_app(app, JS_RUNNER_APP, furi_string_get_cstr(path));
        } else {
            loader_applications_start_app(app, furi_string_get_cstr(path), NULL);
        }
        furi_string_free(path);
    }

    loader_applications_app_free(app);

    if(loader_applications->closed_cb) {
        loader_applications->closed_cb(loader_applications->context);
    }

    return 0;
}
