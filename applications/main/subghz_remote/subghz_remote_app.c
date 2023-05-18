#include "subghz_remote_app_i.h"

#include <lib/subghz/protocols/protocol_items.h>

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

    // Enable power for External CC1101 if it is connected
    furi_hal_subghz_enable_ext_power();
    // Auto switch to internal radio if external radio is not available
    furi_delay_ms(15);
    if(!furi_hal_subghz_check_radio()) {
        furi_hal_subghz_select_radio_type(SubGhzRadioInternal);
        furi_hal_subghz_init_radio_type(SubGhzRadioInternal);
    }

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
        app->view_dispatcher, SubRemViewSubmenu, submenu_get_view(app->submenu));

    //Dialog
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    // Remote view
    app->subrem_remote_view = subrem_view_remote_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubRemViewIDRemote,
        subrem_view_remote_get_view(app->subrem_remote_view));

    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        app->subs_preset[i] = subrem_sub_file_preset_alloc();
    }

    app->setting = subghz_setting_alloc();
    subghz_setting_load(app->setting, EXT_PATH("subghz/assets/setting_user"));

    app->environment = subghz_environment_alloc();

    subghz_environment_load_keystore(app->environment, EXT_PATH("subghz/assets/keeloq_mfcodes"));
    subghz_environment_load_keystore(
        app->environment, EXT_PATH("subghz/assets/keeloq_mfcodes_user"));
    subghz_environment_set_came_atomo_rainbow_table_file_name(
        app->environment, EXT_PATH("subghz/assets/came_atomo"));
    subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
        app->environment, EXT_PATH("subghz/assets/alutech_at_4n"));
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        app->environment, EXT_PATH("subghz/assets/nice_flor_s"));
    subghz_environment_set_protocol_registry(app->environment, (void*)&subghz_protocol_registry);

    app->receiver = subghz_receiver_alloc_init(app->environment);

    app->tx_running = false;

#ifdef SUBREM_LIGHT
    scene_manager_next_scene(app->scene_manager, SubRemSceneOpenMapFile);
#else
    scene_manager_next_scene(app->scene_manager, SubRemSceneStart);
#endif

    return app;
}

void subghz_remote_app_free(SubGhzRemoteApp* app) {
    furi_assert(app);

    furi_hal_power_suppress_charge_exit();

    // Disable power for External CC1101 if it was enabled and module is connected
    furi_hal_subghz_disable_ext_power();
    // Reinit SPI handles for internal radio / nfc
    furi_hal_subghz_init_radio_type(SubGhzRadioInternal);

    // Submenu
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewSubmenu);
    submenu_free(app->submenu);

    //Dialog
    furi_record_close(RECORD_DIALOGS);

    // Remote view
    view_dispatcher_remove_view(app->view_dispatcher, SubRemViewIDRemote);
    subrem_view_remote_free(app->subrem_remote_view);

    subghz_receiver_free(app->receiver);
    subghz_environment_free(app->environment);
    subghz_setting_free(app->setting);

    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        subrem_sub_file_preset_free(app->subs_preset[i]);
    }

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Close records
    furi_record_close(RECORD_GUI);

    // Path strings
    furi_string_free(app->file_path);

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
