#include "../../lightmeter.h"

static void lightmeter_scene_main_on_left(void* context) {
    LightMeterApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, LightMeterAppCustomEventConfig);
}

static void lightmeter_scene_main_on_right(void* context) {
    LightMeterApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, LightMeterAppCustomEventReset);
}

void lightmeter_scene_main_on_enter(void* context) {
    LightMeterApp* app = context;

    variable_item_list_reset(app->var_item_list);
    main_view_set_iso(app->main_view, app->config->iso);
    main_view_set_nd(app->main_view, app->config->nd);
    main_view_set_dome(app->main_view, app->config->dome);
    main_view_set_lux_only(app->main_view, app->config->lux_only);
    main_view_set_measurement_resolution(app->main_view, app->config->measurement_resolution);

    lightmeter_main_view_set_left_callback(app->main_view, lightmeter_scene_main_on_left, app);
    lightmeter_main_view_set_right_callback(app->main_view, lightmeter_scene_main_on_right, app);
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
        } else if(event.event == LightMeterAppCustomEventReset) {
            lightmeter_app_reset_callback(app);
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
    UNUSED(context);
}
