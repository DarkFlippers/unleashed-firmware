#include "assets_icons.h"
#include "file_browser_app_i.h"
#include "gui/modules/file_browser.h"
#include "m-string.h"
#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <lib/toolbox/path.h>

static bool file_browser_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    FileBrowserApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool file_browser_app_back_event_callback(void* context) {
    furi_assert(context);
    FileBrowserApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void file_browser_app_tick_event_callback(void* context) {
    furi_assert(context);
    FileBrowserApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

FileBrowserApp* file_browser_app_alloc(char* arg) {
    UNUSED(arg);
    FileBrowserApp* app = malloc(sizeof(FileBrowserApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);

    app->scene_manager = scene_manager_alloc(&file_browser_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, file_browser_app_tick_event_callback, 500);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, file_browser_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, file_browser_app_back_event_callback);

    app->widget = widget_alloc();

    string_init(app->file_path);
    app->file_browser = file_browser_alloc(app->file_path);
    file_browser_configure(app->file_browser, "*", true, &I_badusb_10px, true);

    view_dispatcher_add_view(
        app->view_dispatcher, FileBrowserAppViewStart, widget_get_view(app->widget));
    view_dispatcher_add_view(
        app->view_dispatcher, FileBrowserAppViewResult, widget_get_view(app->widget));
    view_dispatcher_add_view(
        app->view_dispatcher, FileBrowserAppViewBrowser, file_browser_get_view(app->file_browser));

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    scene_manager_next_scene(app->scene_manager, FileBrowserSceneStart);

    return app;
}

void file_browser_app_free(FileBrowserApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, FileBrowserAppViewStart);
    view_dispatcher_remove_view(app->view_dispatcher, FileBrowserAppViewResult);
    view_dispatcher_remove_view(app->view_dispatcher, FileBrowserAppViewBrowser);
    widget_free(app->widget);
    file_browser_free(app->file_browser);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Close records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);

    string_clear(app->file_path);

    free(app);
}

int32_t file_browser_app(void* p) {
    FileBrowserApp* file_browser_app = file_browser_app_alloc((char*)p);

    view_dispatcher_run(file_browser_app->view_dispatcher);

    file_browser_app_free(file_browser_app);
    return 0;
}
