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

SubGhzRemoteApp* subghz_remote_app_alloc() {
    SubGhzRemoteApp* app = malloc(sizeof(SubGhzRemoteApp));

    // // Enable 5v power, multiple attempts to avoid issues with power chip protection false triggering
    // uint8_t attempts = 0;
    // while(!furi_hal_power_is_otg_enabled() && attempts++ < 5) {
    //     furi_hal_power_enable_otg();
    //     furi_delay_ms(10);
    // }

    app->file_path = furi_string_alloc();
    furi_string_set(app->file_path, STORAGE_APP_DATA_PATH_PREFIX);
    //app->error = SubGhzRemoteErrorNoError;

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
        app->view_dispatcher, SubRemViewSubmenu, submenu_get_view(app->submenu));

    // Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, SubRemViewWidget, widget_get_view(app->widget));

    // Text Input
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SubRemViewTextInput, text_input_get_view(app->text_input));

    // Popup
    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, SubRemViewPopup, popup_get_view(app->popup));

    //Dialog
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    // Remote view
    app->subrem_remote_view = subrem_view_remote_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubRemViewIDRemote,
        subrem_view_remote_get_view(app->subrem_remote_view));
    /*
    // Reader view
    app->subghz_remote_reader_view = subghz_remote_reader_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubRemViewReader,
        subghz_remote_reader_view_get_view(app->subghz_remote_reader_view));

    // Writer view
    app->subghz_remote_writer_view = subghz_remote_writer_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubRemViewWriter,
        subghz_remote_writer_view_get_view(app->subghz_remote_writer_view));

    // Chip detect view
    app->subghz_remote_chip_detect_view = subghz_remote_chip_detect_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubRemViewChipDetect,
        subghz_remote_chip_detect_view_get_view(app->subghz_remote_chip_detect_view));
*/
    for(uint8_t i = 0; i < SUBREM_MAX_SUB_KEY_COUNT; i++) {
        app->subs_preset[i] = subrem_sub_file_preset_alloc();
    }

    app->setting = subghz_setting_alloc();
    subghz_setting_load(app->setting, EXT_PATH("subghz/assets/setting_user"));

    scene_manager_next_scene(app->scene_manager, SubRemSceneStart);

    return app;
}

void subghz_remote_app_free(SubGhzRemoteApp* app) {
    furi_assert(app);

    // Submenu
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewSubmenu);
    submenu_free(app->submenu);

    //  Widget
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewWidget);
    widget_free(app->widget);

    // TextInput
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewTextInput);
    text_input_free(app->text_input);

    // Popup
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewPopup);
    popup_free(app->popup);

    //Dialog
    furi_record_close(RECORD_DIALOGS);

    // Remote view
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewIDRemote);
    subrem_view_remote_free(app->subrem_remote_view);

    subghz_setting_free(app->setting);

    for(uint8_t i = 0; i < SUBREM_MAX_SUB_KEY_COUNT; i++) {
        subrem_sub_file_preset_free(app->subs_preset[i]);
    }

    // // Reader view
    // view_dispatcher_remove_view(app->view_dispatcher, SubRemViewReader);
    // subghz_remote_reader_view_free(app->subghz_remote_reader_view);

    // // Writer view
    // view_dispatcher_remove_view(app->view_dispatcher, SubRemViewWriter);
    // subghz_remote_writer_view_free(app->subghz_remote_writer_view);

    // // Chip detect view
    // view_dispatcher_remove_view(app->view_dispatcher, SubRemViewChipDetect);
    // subghz_remote_chip_detect_view_free(app->subghz_remote_chip_detect_view);

    // // View dispatcher
    // view_dispatcher_free(app->view_dispatcher);
    // scene_manager_free(app->scene_manager);

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Close records
    furi_record_close(RECORD_GUI);

    // Path strings
    furi_string_free(app->file_path);

    // Disable 5v power
    // if(furi_hal_power_is_otg_enabled()) {
    //     furi_hal_power_disable_otg();
    // }

    free(app);
}

int32_t subghz_remote_app(void* p) {
    UNUSED(p);
    SubGhzRemoteApp* subghz_remote_app = subghz_remote_app_alloc();

    furi_string_set(subghz_remote_app->file_path, SUBREM_APP_FOLDER);

    view_dispatcher_run(subghz_remote_app->view_dispatcher);

    subghz_remote_app_free(subghz_remote_app);

    return 0;
}
