#include "subghz_remote_app_i.h"

static bool subghz_remote_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool subghz_remote_app_back_event_callback(void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void subghz_remote_app_tick_event_callback(void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static void subghz_remote_make_app_folder(SubGhzRemoteApp* app) {
    furi_assert(app);

    Storage* storage = furi_record_open(RECORD_STORAGE);

    // Migrate old users data
    storage_common_migrate(storage, EXT_PATH("unirf"), SUBREM_APP_FOLDER);

    if(!storage_simply_mkdir(storage, SUBREM_APP_FOLDER)) {
        // FURI_LOG_E(TAG, "Could not create folder %s", SUBREM_APP_FOLDER);
        dialog_message_show_storage_error(app->dialogs, "Cannot create\napp folder");
    }
    furi_record_close(RECORD_STORAGE);
}

SubGhzRemoteApp* subghz_remote_app_alloc() {
    SubGhzRemoteApp* app = malloc(sizeof(SubGhzRemoteApp));

    furi_hal_power_suppress_charge_enter();

    app->file_path = furi_string_alloc();
    furi_string_set(app->file_path, SUBREM_APP_FOLDER);

    // GUI
    app->gui = furi_record_open(RECORD_GUI);

    // View Dispatcher
    app->view_dispatcher = view_dispatcher_alloc();

    app->scene_manager = scene_manager_alloc(&subrem_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, subghz_remote_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, subghz_remote_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, subghz_remote_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Open Notification record
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // SubMenu
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SubRemViewIDSubmenu, submenu_get_view(app->submenu));

    // Dialog
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    // TextInput
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SubRemViewIDTextInput, text_input_get_view(app->text_input));

    // Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SubRemViewIDWidget, widget_get_view(app->widget));

    // Popup
    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, SubRemViewIDPopup, popup_get_view(app->popup));

    // Remote view
    app->subrem_remote_view = subrem_view_remote_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubRemViewIDRemote,
        subrem_view_remote_get_view(app->subrem_remote_view));

    // Edit Menu view
    app->subrem_edit_menu = subrem_view_edit_menu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubRemViewIDEditMenu,
        subrem_view_edit_menu_get_view(app->subrem_edit_menu));

    app->map_preset = malloc(sizeof(SubRemMapPreset));
    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        app->map_preset->subs_preset[i] = subrem_sub_file_preset_alloc();
    }

    app->txrx = subghz_txrx_alloc();

    subghz_txrx_set_need_save_callback(app->txrx, subrem_save_active_sub, app);

    app->map_not_saved = false;

    return app;
}

void subghz_remote_app_free(SubGhzRemoteApp* app) {
    furi_assert(app);

    furi_hal_power_suppress_charge_exit();

    // Submenu
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewIDSubmenu);
    submenu_free(app->submenu);

    // Dialog
    furi_record_close(RECORD_DIALOGS);

    // TextInput
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewIDTextInput);
    text_input_free(app->text_input);

    // Widget
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewIDWidget);
    widget_free(app->widget);

    // Popup
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewIDPopup);
    popup_free(app->popup);

    // Remote view
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewIDRemote);
    subrem_view_remote_free(app->subrem_remote_view);

    // Edit view
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewIDEditMenu);
    subrem_view_edit_menu_free(app->subrem_edit_menu);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    subghz_txrx_free(app->txrx);

    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        subrem_sub_file_preset_free(app->map_preset->subs_preset[i]);
    }
    free(app->map_preset);

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Close records
    furi_record_close(RECORD_GUI);

    // Path strings
    furi_string_free(app->file_path);

    free(app);
}

int32_t subghz_remote_app(void* arg) {
    SubGhzRemoteApp* subghz_remote_app = subghz_remote_app_alloc();

    subghz_remote_make_app_folder(subghz_remote_app);

    bool map_loaded = false;

    if((arg != NULL) && (strlen(arg) != 0)) {
        furi_string_set(subghz_remote_app->file_path, (const char*)arg);
        SubRemLoadMapState load_state = subrem_map_file_load(
            subghz_remote_app, furi_string_get_cstr(subghz_remote_app->file_path));

        if(load_state == SubRemLoadMapStateOK || load_state == SubRemLoadMapStateNotAllOK) {
            map_loaded = true;
        } else {
            // TODO Replace
            dialog_message_show_storage_error(subghz_remote_app->dialogs, "Cannot load\nmap file");
        }
    }

    if(map_loaded) {
        scene_manager_next_scene(subghz_remote_app->scene_manager, SubRemSceneRemote);
    } else {
        furi_string_set(subghz_remote_app->file_path, SUBREM_APP_FOLDER);
        scene_manager_next_scene(subghz_remote_app->scene_manager, SubRemSceneStart);
        scene_manager_next_scene(subghz_remote_app->scene_manager, SubRemSceneOpenMapFile);
    }

    view_dispatcher_run(subghz_remote_app->view_dispatcher);

    subghz_remote_app_free(subghz_remote_app);

    return 0;
}
