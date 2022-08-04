#include "bad_usb_app_i.h"
#include "bad_usb_settings_filename.h"
#include "m-string.h"
#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <lib/toolbox/path.h>

#define BAD_USB_SETTINGS_PATH BAD_USB_APP_BASE_FOLDER "/" BAD_USB_SETTINGS_FILE_NAME

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

static void bad_usb_load_settings(BadUsbApp* app) {
    File* settings_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    if(storage_file_open(settings_file, BAD_USB_SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char chr;
        while((storage_file_read(settings_file, &chr, 1) == 1) &&
              !storage_file_eof(settings_file) && !isspace(chr)) {
            string_push_back(app->keyboard_layout, chr);
        }
    }
    storage_file_close(settings_file);
    storage_file_free(settings_file);
}

static void bad_usb_save_settings(BadUsbApp* app) {
    File* settings_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    if(storage_file_open(settings_file, BAD_USB_SETTINGS_PATH, FSAM_WRITE, FSOM_OPEN_ALWAYS)) {
        storage_file_write(
            settings_file,
            string_get_cstr(app->keyboard_layout),
            string_size(app->keyboard_layout));
        storage_file_write(settings_file, "\n", 1);
    }
    storage_file_close(settings_file);
    storage_file_free(settings_file);
}

BadUsbApp* bad_usb_app_alloc(char* arg) {
    BadUsbApp* app = malloc(sizeof(BadUsbApp));

    app->bad_usb_script = NULL;

    string_init(app->file_path);
    string_init(app->keyboard_layout);
    if(arg && strlen(arg)) {
        string_set_str(app->file_path, arg);
    }

    bad_usb_load_settings(app);

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->dialogs = furi_record_open(RECORD_DIALOGS);

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

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadUsbAppViewConfig, submenu_get_view(app->submenu));

    app->bad_usb_view = bad_usb_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadUsbAppViewWork, bad_usb_get_view(app->bad_usb_view));

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    if(furi_hal_usb_is_locked()) {
        app->error = BadUsbAppErrorCloseRpc;
        scene_manager_next_scene(app->scene_manager, BadUsbSceneError);
    } else {
        if(!string_empty_p(app->file_path)) {
            scene_manager_next_scene(app->scene_manager, BadUsbSceneWork);
        } else {
            string_set_str(app->file_path, BAD_USB_APP_BASE_FOLDER);
            scene_manager_next_scene(app->scene_manager, BadUsbSceneFileSelect);
        }
    }

    return app;
}

void bad_usb_app_free(BadUsbApp* app) {
    furi_assert(app);

    if(app->bad_usb_script != NULL) {
        bad_usb_script_close(app->bad_usb_script);
        app->bad_usb_script = NULL;
    }

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewWork);

    bad_usb_free(app->bad_usb_view);

    // Custom Widget
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewError);
    widget_free(app->widget);

    // Submenu
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewConfig);
    submenu_free(app->submenu);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Close records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);

    bad_usb_save_settings(app);

    string_clear(app->file_path);
    string_clear(app->keyboard_layout);

    free(app);
}

int32_t bad_usb_app(void* p) {
    BadUsbApp* bad_usb_app = bad_usb_app_alloc((char*)p);

    view_dispatcher_run(bad_usb_app->view_dispatcher);

    bad_usb_app_free(bad_usb_app);
    return 0;
}