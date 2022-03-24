#include "bad_usb_app_i.h"
#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <lib/toolbox/path.h>

static bool bad_usb_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    BadUsbApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool bad_usb_app_back_event_callback(void* context) {
    furi_assert(context);
    BadUsbApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void bad_usb_app_tick_event_callback(void* context) {
    furi_assert(context);
    BadUsbApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static bool bad_usb_check_assets() {
    Storage* fs_api = furi_record_open("storage");

    File* dir = storage_file_alloc(fs_api);
    bool ret = false;

    if(storage_dir_open(dir, BAD_USB_APP_PATH_FOLDER)) {
        ret = true;
    }

    storage_dir_close(dir);
    storage_file_free(dir);

    furi_record_close("storage");

    return ret;
}

BadUsbApp* bad_usb_app_alloc(char* arg) {
    BadUsbApp* app = malloc(sizeof(BadUsbApp));

    if(arg != NULL) {
        string_t filename;
        string_init(filename);
        path_extract_filename_no_ext(arg, filename);
        strncpy(app->file_name, string_get_cstr(filename), BAD_USB_FILE_NAME_LEN);
        string_clear(filename);
    }

    app->gui = furi_record_open("gui");
    app->notifications = furi_record_open("notification");
    app->dialogs = furi_record_open("dialogs");

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);

    app->scene_manager = scene_manager_alloc(&bad_usb_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, bad_usb_app_tick_event_callback, 500);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, bad_usb_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, bad_usb_app_back_event_callback);

    // Custom Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadUsbAppViewError, widget_get_view(app->widget));

    app->bad_usb_view = bad_usb_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadUsbAppViewWork, bad_usb_get_view(app->bad_usb_view));

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    if(furi_hal_usb_is_locked()) {
        app->error = BadUsbAppErrorCloseRpc;
        scene_manager_next_scene(app->scene_manager, BadUsbSceneError);
    } else {
        if(*app->file_name != '\0') {
            scene_manager_next_scene(app->scene_manager, BadUsbSceneWork);
        } else if(bad_usb_check_assets()) {
            scene_manager_next_scene(app->scene_manager, BadUsbSceneFileSelect);
        } else {
            app->error = BadUsbAppErrorNoFiles;
            scene_manager_next_scene(app->scene_manager, BadUsbSceneError);
        }
    }

    return app;
}

void bad_usb_app_free(BadUsbApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewFileSelect);
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewWork);
    bad_usb_free(app->bad_usb_view);

    // Custom Widget
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewError);
    widget_free(app->widget);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Close records
    furi_record_close("gui");
    furi_record_close("notification");
    furi_record_close("dialogs");

    free(app);
}

int32_t bad_usb_app(void* p) {
    BadUsbApp* bad_usb_app = bad_usb_app_alloc((char*)p);

    view_dispatcher_run(bad_usb_app->view_dispatcher);

    bad_usb_app_free(bad_usb_app);
    return 0;
}
