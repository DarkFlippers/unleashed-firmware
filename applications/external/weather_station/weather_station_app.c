#include "weather_station_app_i.h"

#include <furi.h>
#include <furi_hal.h>
#include "protocols/protocol_items.h"

static bool weather_station_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    WeatherStationApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool weather_station_app_back_event_callback(void* context) {
    furi_assert(context);
    WeatherStationApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void weather_station_app_tick_event_callback(void* context) {
    furi_assert(context);
    WeatherStationApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

WeatherStationApp* weather_station_app_alloc() {
    WeatherStationApp* app = malloc(sizeof(WeatherStationApp));

    // GUI
    app->gui = furi_record_open(RECORD_GUI);

    // View Dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&weather_station_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, weather_station_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, weather_station_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, weather_station_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Open Notification record
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // Variable Item List
    app->variable_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        WeatherStationViewVariableItemList,
        variable_item_list_get_view(app->variable_item_list));

    // SubMenu
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, WeatherStationViewSubmenu, submenu_get_view(app->submenu));

    // Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, WeatherStationViewWidget, widget_get_view(app->widget));

    // Receiver
    app->ws_receiver = ws_view_receiver_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        WeatherStationViewReceiver,
        ws_view_receiver_get_view(app->ws_receiver));

    // Receiver Info
    app->ws_receiver_info = ws_view_receiver_info_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        WeatherStationViewReceiverInfo,
        ws_view_receiver_info_get_view(app->ws_receiver_info));

    //init setting
    app->setting = subghz_setting_alloc();

    //ToDo FIX  file name setting
    subghz_setting_load(app->setting, EXT_PATH("subghz/assets/setting_user"));

    //init Worker & Protocol & History
    app->lock = WSLockOff;
    app->txrx = malloc(sizeof(WeatherStationTxRx));
    app->txrx->preset = malloc(sizeof(SubGhzRadioPreset));
    app->txrx->preset->name = furi_string_alloc();
    ws_preset_init(app, "AM650", subghz_setting_get_default_frequency(app->setting), NULL, 0);

    app->txrx->hopper_state = WSHopperStateOFF;
    app->txrx->history = ws_history_alloc();
    app->txrx->worker = subghz_worker_alloc();
    app->txrx->environment = subghz_environment_alloc();
    subghz_environment_set_protocol_registry(
        app->txrx->environment, (void*)&weather_station_protocol_registry);
    app->txrx->receiver = subghz_receiver_alloc_init(app->txrx->environment);

    subghz_receiver_set_filter(app->txrx->receiver, SubGhzProtocolFlag_Decodable);
    subghz_worker_set_overrun_callback(
        app->txrx->worker, (SubGhzWorkerOverrunCallback)subghz_receiver_reset);
    subghz_worker_set_pair_callback(
        app->txrx->worker, (SubGhzWorkerPairCallback)subghz_receiver_decode);
    subghz_worker_set_context(app->txrx->worker, app->txrx->receiver);

    furi_hal_power_suppress_charge_enter();

    scene_manager_next_scene(app->scene_manager, WeatherStationSceneStart);

    return app;
}

void weather_station_app_free(WeatherStationApp* app) {
    furi_assert(app);

    //CC1101 off
    ws_sleep(app);

    // Submenu
    view_dispatcher_remove_view(app->view_dispatcher, WeatherStationViewSubmenu);
    submenu_free(app->submenu);

    // Variable Item List
    view_dispatcher_remove_view(app->view_dispatcher, WeatherStationViewVariableItemList);
    variable_item_list_free(app->variable_item_list);

    //  Widget
    view_dispatcher_remove_view(app->view_dispatcher, WeatherStationViewWidget);
    widget_free(app->widget);

    // Receiver
    view_dispatcher_remove_view(app->view_dispatcher, WeatherStationViewReceiver);
    ws_view_receiver_free(app->ws_receiver);

    // Receiver Info
    view_dispatcher_remove_view(app->view_dispatcher, WeatherStationViewReceiverInfo);
    ws_view_receiver_info_free(app->ws_receiver_info);

    //setting
    subghz_setting_free(app->setting);

    //Worker & Protocol & History
    subghz_receiver_free(app->txrx->receiver);
    subghz_environment_free(app->txrx->environment);
    ws_history_free(app->txrx->history);
    subghz_worker_free(app->txrx->worker);
    furi_string_free(app->txrx->preset->name);
    free(app->txrx->preset);
    free(app->txrx);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Close records
    furi_record_close(RECORD_GUI);

    furi_hal_power_suppress_charge_exit();

    free(app);
}

int32_t weather_station_app(void* p) {
    UNUSED(p);
    WeatherStationApp* weather_station_app = weather_station_app_alloc();

    view_dispatcher_run(weather_station_app->view_dispatcher);

    weather_station_app_free(weather_station_app);

    return 0;
}
