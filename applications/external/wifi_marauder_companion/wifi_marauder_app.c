#include "wifi_marauder_app_i.h"

#include <furi.h>
#include <furi_hal.h>

static bool wifi_marauder_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    WifiMarauderApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool wifi_marauder_app_back_event_callback(void* context) {
    furi_assert(context);
    WifiMarauderApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void wifi_marauder_app_tick_event_callback(void* context) {
    furi_assert(context);
    WifiMarauderApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

WifiMarauderApp* wifi_marauder_app_alloc() {
    WifiMarauderApp* app = malloc(sizeof(WifiMarauderApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->capture_file = storage_file_alloc(app->storage);
    app->log_file = storage_file_alloc(app->storage);
    app->save_pcap_setting_file = storage_file_alloc(app->storage);
    app->save_logs_setting_file = storage_file_alloc(app->storage);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&wifi_marauder_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, wifi_marauder_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, wifi_marauder_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, wifi_marauder_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        WifiMarauderAppViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    for(int i = 0; i < NUM_MENU_ITEMS; ++i) {
        app->selected_option_index[i] = 0;
    }

    app->special_case_input_step = 0;

    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, WifiMarauderAppViewConsoleOutput, text_box_get_view(app->text_box));
    app->text_box_store = furi_string_alloc();
    furi_string_reserve(app->text_box_store, WIFI_MARAUDER_TEXT_BOX_STORE_SIZE);

    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, WifiMarauderAppViewTextInput, text_input_get_view(app->text_input));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, WifiMarauderAppViewWidget, widget_get_view(app->widget));

    app->has_saved_logs_this_session = false;

    // if user hasn't confirmed whether to save pcaps and logs to sdcard, then prompt when scene starts
    app->need_to_prompt_settings_init =
        (!storage_file_exists(app->storage, SAVE_PCAP_SETTING_FILEPATH) ||
         !storage_file_exists(app->storage, SAVE_LOGS_SETTING_FILEPATH));

    scene_manager_next_scene(app->scene_manager, WifiMarauderSceneStart);

    return app;
}

void wifi_marauder_make_app_folder(WifiMarauderApp* app) {
    furi_assert(app);

    if(!storage_simply_mkdir(app->storage, MARAUDER_APP_FOLDER)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot create\napp folder");
    }

    if(!storage_simply_mkdir(app->storage, MARAUDER_APP_FOLDER_PCAPS)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot create\npcaps folder");
    }

    if(!storage_simply_mkdir(app->storage, MARAUDER_APP_FOLDER_LOGS)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot create\npcaps folder");
    }
}

void wifi_marauder_load_settings(WifiMarauderApp* app) {
    if(storage_file_open(
           app->save_pcap_setting_file,
           SAVE_PCAP_SETTING_FILEPATH,
           FSAM_READ,
           FSOM_OPEN_EXISTING)) {
        char ok[1];
        storage_file_read(app->save_pcap_setting_file, ok, sizeof(ok));
        app->ok_to_save_pcaps = ok[0] == 'Y';
    }
    storage_file_close(app->save_pcap_setting_file);

    if(storage_file_open(
           app->save_logs_setting_file,
           SAVE_LOGS_SETTING_FILEPATH,
           FSAM_READ,
           FSOM_OPEN_EXISTING)) {
        char ok[1];
        storage_file_read(app->save_logs_setting_file, ok, sizeof(ok));
        app->ok_to_save_logs = ok[0] == 'Y';
    }
    storage_file_close(app->save_logs_setting_file);
}

void wifi_marauder_app_free(WifiMarauderApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, WifiMarauderAppViewVarItemList);
    view_dispatcher_remove_view(app->view_dispatcher, WifiMarauderAppViewConsoleOutput);
    view_dispatcher_remove_view(app->view_dispatcher, WifiMarauderAppViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, WifiMarauderAppViewWidget);
    widget_free(app->widget);
    text_box_free(app->text_box);
    furi_string_free(app->text_box_store);
    text_input_free(app->text_input);
    storage_file_free(app->capture_file);
    storage_file_free(app->log_file);
    storage_file_free(app->save_pcap_setting_file);
    storage_file_free(app->save_logs_setting_file);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    wifi_marauder_uart_free(app->uart);
    wifi_marauder_uart_free(app->lp_uart);

    // Close records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_DIALOGS);

    free(app);
}

int32_t wifi_marauder_app(void* p) {
    UNUSED(p);

    uint8_t attempts = 0;
    while(!furi_hal_power_is_otg_enabled() && attempts++ < 5) {
        furi_hal_power_enable_otg();
        furi_delay_ms(10);
    }
    furi_delay_ms(200);

    WifiMarauderApp* wifi_marauder_app = wifi_marauder_app_alloc();

    wifi_marauder_make_app_folder(wifi_marauder_app);
    wifi_marauder_load_settings(wifi_marauder_app);

    wifi_marauder_app->uart = wifi_marauder_usart_init(wifi_marauder_app);
    wifi_marauder_app->lp_uart = wifi_marauder_lp_uart_init(wifi_marauder_app);

    view_dispatcher_run(wifi_marauder_app->view_dispatcher);

    wifi_marauder_app_free(wifi_marauder_app);

    if(furi_hal_power_is_otg_enabled()) {
        furi_hal_power_disable_otg();
    }

    return 0;
}
