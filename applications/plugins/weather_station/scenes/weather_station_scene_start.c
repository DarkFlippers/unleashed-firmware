#include "../weather_station_app_i.h"

typedef enum {
    SubmenuIndexWeatherStationReceiver,
    SubmenuIndexWeatherStationAbout,
} SubmenuIndex;

void weather_station_scene_start_submenu_callback(void* context, uint32_t index) {
    WeatherStationApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void weather_station_scene_start_on_enter(void* context) {
    UNUSED(context);
    WeatherStationApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Read Weather Station",
        SubmenuIndexWeatherStationReceiver,
        weather_station_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "About",
        SubmenuIndexWeatherStationAbout,
        weather_station_scene_start_submenu_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, WeatherStationSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, WeatherStationViewSubmenu);
}

bool weather_station_scene_start_on_event(void* context, SceneManagerEvent event) {
    WeatherStationApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexWeatherStationAbout) {
            scene_manager_next_scene(app->scene_manager, WeatherStationSceneAbout);
            consumed = true;
        } else if(event.event == SubmenuIndexWeatherStationReceiver) {
            scene_manager_next_scene(app->scene_manager, WeatherStationSceneReceiver);
            consumed = true;
        }
        scene_manager_set_scene_state(app->scene_manager, WeatherStationSceneStart, event.event);
    }

    return consumed;
}

void weather_station_scene_start_on_exit(void* context) {
    WeatherStationApp* app = context;
    submenu_reset(app->submenu);
}
