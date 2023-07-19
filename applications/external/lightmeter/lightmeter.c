#include "lightmeter.h"
#include "lightmeter_helper.h"

#define TAG "MAIN APP"

static bool lightmeter_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    LightMeterApp* app = context;

    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool lightmeter_back_event_callback(void* context) {
    furi_assert(context);
    LightMeterApp* app = context;

    return scene_manager_handle_back_event(app->scene_manager);
}

static void lightmeter_tick_event_callback(void* context) {
    furi_assert(context);
    LightMeterApp* app = context;

    scene_manager_handle_tick_event(app->scene_manager);
}

LightMeterApp* lightmeter_app_alloc(uint32_t first_scene) {
    LightMeterApp* app = malloc(sizeof(LightMeterApp));

    // Set default values to config
    app->config = malloc(sizeof(LightMeterConfig));
    app->config->iso = DEFAULT_ISO;
    app->config->nd = DEFAULT_ND;
    app->config->aperture = DEFAULT_APERTURE;
    app->config->dome = DEFAULT_DOME;
    app->config->backlight = DEFAULT_BACKLIGHT;

    // Records
    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // View dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&lightmeter_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, lightmeter_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, lightmeter_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, lightmeter_tick_event_callback, furi_ms_to_ticks(200));
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Views
    app->main_view = main_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, LightMeterAppViewMainView, main_view_get_view(app->main_view));

    // Set default values to main view from config
    main_view_set_iso(app->main_view, app->config->iso);
    main_view_set_nd(app->main_view, app->config->nd);
    main_view_set_aperture(app->main_view, app->config->aperture);
    main_view_set_speed(app->main_view, DEFAULT_SPEED);
    main_view_set_dome(app->main_view, app->config->dome);

    // Variable item list
    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        LightMeterAppViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    // Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, LightMeterAppViewAbout, widget_get_view(app->widget));
    view_dispatcher_add_view(
        app->view_dispatcher, LightMeterAppViewHelp, widget_get_view(app->widget));

    // Set first scene
    scene_manager_next_scene(app->scene_manager, first_scene);
    return app;
}

void lightmeter_app_free(LightMeterApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, LightMeterAppViewMainView);
    main_view_free(app->main_view);

    // Variable item list
    view_dispatcher_remove_view(app->view_dispatcher, LightMeterAppViewVarItemList);
    variable_item_list_free(app->var_item_list);

    //  Widget
    view_dispatcher_remove_view(app->view_dispatcher, LightMeterAppViewAbout);
    view_dispatcher_remove_view(app->view_dispatcher, LightMeterAppViewHelp);
    widget_free(app->widget);

    // View dispatcher
    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    // Records
    furi_record_close(RECORD_GUI);
    if(app->config->backlight != BACKLIGHT_AUTO) {
        notification_message(
            app->notifications,
            &sequence_display_backlight_enforce_auto); // set backlight back to auto
    }
    furi_record_close(RECORD_NOTIFICATION);

    free(app->config);
    free(app);
}

int32_t lightmeter_app(void* p) {
    UNUSED(p);
    uint32_t first_scene = LightMeterAppSceneMain;
    LightMeterApp* app = lightmeter_app_alloc(first_scene);
    view_dispatcher_run(app->view_dispatcher);
    lightmeter_app_free(app);
    return 0;
}

void lightmeter_app_set_config(LightMeterApp* context, LightMeterConfig* config) {
    LightMeterApp* app = context;

    app->config = config;
}

void lightmeter_app_i2c_init_sensor(LightMeterApp* context) {
    LightMeterApp* app = context;
    switch(app->config->sensor_type) {
    case SENSOR_BH1750:
        bh1750_set_power_state(1);
        bh1750_init();
        bh1750_set_mode(ONETIME_HIGH_RES_MODE);
        break;
    case SENSOR_MAX44009:
        max44009_init();
        break;
    default:
        FURI_LOG_E(TAG, "Invalid sensor type %d", app->config->sensor_type);
        return;
    }
}

void lightmeter_app_i2c_deinit_sensor(LightMeterApp* context) {
    LightMeterApp* app = context;
    switch(app->config->sensor_type) {
    case SENSOR_BH1750:
        bh1750_set_power_state(0);
        break;
    case SENSOR_MAX44009:
        // nothing
        break;
    default:
        FURI_LOG_E(TAG, "Invalid sensor type %d", app->config->sensor_type);
        return;
    }
}

void lightmeter_app_i2c_callback(LightMeterApp* context) {
    LightMeterApp* app = context;

    float EV = 0;
    float lux = 0;
    bool response = 0;

    if(app->config->sensor_type == SENSOR_BH1750) {
        if(bh1750_trigger_manual_conversion() == BH1750_OK) {
            bh1750_read_light(&lux);
            response = 1;
        }
    } else if(app->config->sensor_type == SENSOR_MAX44009) {
        if(max44009_read_light(&lux)) response = 1;
    }

    if(main_view_get_dome(app->main_view)) lux *= DOME_COEFFICIENT;
    EV = lux2ev(lux);

    main_view_set_lux(app->main_view, lux);
    main_view_set_EV(app->main_view, EV);
    main_view_set_response(app->main_view, response);
}
