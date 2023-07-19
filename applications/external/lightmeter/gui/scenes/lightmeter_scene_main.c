#include "../../lightmeter.h"

static void lightmeter_scene_main_on_left(void* context) {
    LightMeterApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, LightMeterAppCustomEventConfig);
}

void lightmeter_scene_main_on_enter(void* context) {
    LightMeterApp* app = context;

    lightmeter_app_i2c_init_sensor(context);
    lightmeter_main_view_set_left_callback(app->main_view, lightmeter_scene_main_on_left, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, LightMeterAppViewMainView);
}

bool lightmeter_scene_main_on_event(void* context, SceneManagerEvent event) {
    LightMeterApp* app = context;

    bool response = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        if(event.event == LightMeterAppCustomEventConfig) {
            scene_manager_next_scene(app->scene_manager, LightMeterAppSceneConfig);
            response = true;
        }
        break;

    case SceneManagerEventTypeTick:
        lightmeter_app_i2c_callback(app);
        response = true;
        break;

    default:
        break;
    }

    return response;
}

void lightmeter_scene_main_on_exit(void* context) {
    lightmeter_app_i2c_deinit_sensor(context);
}
