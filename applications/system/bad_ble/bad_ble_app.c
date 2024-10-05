#include "bad_ble_app_i.h"
#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <lib/toolbox/path.h>
#include <flipper_format/flipper_format.h>

#define BAD_BLE_SETTINGS_PATH           BAD_BLE_APP_BASE_FOLDER "/.badble.settings"
#define BAD_BLE_SETTINGS_FILE_TYPE      "Flipper BadBLE Settings File"
#define BAD_BLE_SETTINGS_VERSION        1
#define BAD_BLE_SETTINGS_DEFAULT_LAYOUT BAD_BLE_APP_PATH_LAYOUT_FOLDER "/en-US.kl"

static bool bad_ble_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    BadBleApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool bad_ble_app_back_event_callback(void* context) {
    furi_assert(context);
    BadBleApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void bad_ble_app_tick_event_callback(void* context) {
    furi_assert(context);
    BadBleApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static void bad_ble_load_settings(BadBleApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff = flipper_format_file_alloc(storage);
    bool state = false;

    FuriString* temp_str = furi_string_alloc();
    uint32_t version = 0;

    if(flipper_format_file_open_existing(fff, BAD_BLE_SETTINGS_PATH)) {
        do {
            if(!flipper_format_read_header(fff, temp_str, &version)) break;
            if((strcmp(furi_string_get_cstr(temp_str), BAD_BLE_SETTINGS_FILE_TYPE) != 0) ||
               (version != BAD_BLE_SETTINGS_VERSION))
                break;

            if(!flipper_format_read_string(fff, "layout", temp_str)) break;

            state = true;
        } while(0);
    }
    flipper_format_free(fff);
    furi_record_close(RECORD_STORAGE);

    if(state) {
        furi_string_set(app->keyboard_layout, temp_str);

        Storage* fs_api = furi_record_open(RECORD_STORAGE);
        FileInfo layout_file_info;
        FS_Error file_check_err = storage_common_stat(
            fs_api, furi_string_get_cstr(app->keyboard_layout), &layout_file_info);
        furi_record_close(RECORD_STORAGE);
        if((file_check_err != FSE_OK) || (layout_file_info.size != 256)) {
            furi_string_set(app->keyboard_layout, BAD_BLE_SETTINGS_DEFAULT_LAYOUT);
        }
    } else {
        furi_string_set(app->keyboard_layout, BAD_BLE_SETTINGS_DEFAULT_LAYOUT);
    }

    furi_string_free(temp_str);
}

static void bad_ble_save_settings(BadBleApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff = flipper_format_file_alloc(storage);

    if(flipper_format_file_open_always(fff, BAD_BLE_SETTINGS_PATH)) {
        do {
            if(!flipper_format_write_header_cstr(
                   fff, BAD_BLE_SETTINGS_FILE_TYPE, BAD_BLE_SETTINGS_VERSION))
                break;
            if(!flipper_format_write_string(fff, "layout", app->keyboard_layout)) break;
        } while(0);
    }

    flipper_format_free(fff);
    furi_record_close(RECORD_STORAGE);
}

BadBleApp* bad_ble_app_alloc(char* arg) {
    BadBleApp* app = malloc(sizeof(BadBleApp));

    app->bad_ble_script = NULL;

    app->file_path = furi_string_alloc();
    app->keyboard_layout = furi_string_alloc();
    if(arg && strlen(arg)) {
        furi_string_set(app->file_path, arg);
    }

    bad_ble_load_settings(app);

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&bad_ble_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, bad_ble_app_tick_event_callback, 500);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, bad_ble_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, bad_ble_app_back_event_callback);

    // Custom Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadBleAppViewWidget, widget_get_view(app->widget));

    // Popup
    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, BadBleAppViewPopup, popup_get_view(app->popup));

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        BadBleAppViewConfig,
        variable_item_list_get_view(app->var_item_list));

    app->bad_ble_view = bad_ble_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadBleAppViewWork, bad_ble_view_get_view(app->bad_ble_view));

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    if(!furi_string_empty(app->file_path)) {
        scene_manager_next_scene(app->scene_manager, BadBleSceneWork);
    } else {
        furi_string_set(app->file_path, BAD_BLE_APP_BASE_FOLDER);
        scene_manager_next_scene(app->scene_manager, BadBleSceneFileSelect);
    }

    return app;
}

void bad_ble_app_free(BadBleApp* app) {
    furi_assert(app);

    if(app->bad_ble_script) {
        bad_ble_script_close(app->bad_ble_script);
        app->bad_ble_script = NULL;
    }

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, BadBleAppViewWork);
    bad_ble_view_free(app->bad_ble_view);

    // Custom Widget
    view_dispatcher_remove_view(app->view_dispatcher, BadBleAppViewWidget);
    widget_free(app->widget);

    // Popup
    view_dispatcher_remove_view(app->view_dispatcher, BadBleAppViewPopup);
    popup_free(app->popup);

    // Config menu
    view_dispatcher_remove_view(app->view_dispatcher, BadBleAppViewConfig);
    variable_item_list_free(app->var_item_list);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Close records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);

    bad_ble_save_settings(app);

    furi_string_free(app->file_path);
    furi_string_free(app->keyboard_layout);

    free(app);
}

int32_t bad_ble_app(void* p) {
    BadBleApp* bad_ble_app = bad_ble_app_alloc((char*)p);

    view_dispatcher_run(bad_ble_app->view_dispatcher);

    bad_ble_app_free(bad_ble_app);
    return 0;
}
