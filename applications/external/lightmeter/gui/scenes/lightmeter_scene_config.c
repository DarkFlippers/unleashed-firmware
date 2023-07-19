#include "../../lightmeter.h"

#define TAG "Scene Config"

static const char* iso_numbers[] = {
    [ISO_6] = "6",
    [ISO_12] = "12",
    [ISO_25] = "25",
    [ISO_50] = "50",
    [ISO_100] = "100",
    [ISO_200] = "200",
    [ISO_400] = "400",
    [ISO_800] = "800",
    [ISO_1600] = "1600",
    [ISO_3200] = "3200",
    [ISO_6400] = "6400",
    [ISO_12800] = "12800",
    [ISO_25600] = "25600",
    [ISO_51200] = "51200",
    [ISO_102400] = "102400",
};

static const char* nd_numbers[] = {
    [ND_0] = "0",
    [ND_2] = "2",
    [ND_4] = "4",
    [ND_8] = "8",
    [ND_16] = "16",
    [ND_32] = "32",
    [ND_64] = "64",
    [ND_128] = "128",
    [ND_256] = "256",
    [ND_512] = "512",
    [ND_1024] = "1024",
    [ND_2048] = "2048",
    [ND_4096] = "4096",
};

static const char* diffusion_dome[] = {
    [WITHOUT_DOME] = "No",
    [WITH_DOME] = "Yes",
};

static const char* backlight[] = {
    [BACKLIGHT_AUTO] = "Auto",
    [BACKLIGHT_ON] = "On",
};

static const char* lux_only[] = {
    [LUX_ONLY_OFF] = "Off",
    [LUX_ONLY_ON] = "On",
};

static const char* sensor_type[] = {
    [SENSOR_BH1750] = "BH1750",
    [SENSOR_MAX44009] = "MAX44009",
};

enum LightMeterSubmenuIndex {
    LightMeterSubmenuIndexISO,
    LightMeterSubmenuIndexND,
    LightMeterSubmenuIndexDome,
    LightMeterSubmenuIndexBacklight,
    LightMeterSubmenuIndexLuxMeter,
    LightMeterSubmenuIndexSensorType,
    LightMeterSubmenuIndexHelp,
    LightMeterSubmenuIndexAbout,
};

static void iso_numbers_cb(VariableItem* item) {
    LightMeterApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, iso_numbers[index]);

    LightMeterConfig* config = app->config;
    config->iso = index;
    lightmeter_app_set_config(app, config);
}

static void nd_numbers_cb(VariableItem* item) {
    LightMeterApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, nd_numbers[index]);

    LightMeterConfig* config = app->config;
    config->nd = index;
    lightmeter_app_set_config(app, config);
}

static void dome_presence_cb(VariableItem* item) {
    LightMeterApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, diffusion_dome[index]);

    LightMeterConfig* config = app->config;
    config->dome = index;
    lightmeter_app_set_config(app, config);
}

static void backlight_cb(VariableItem* item) {
    LightMeterApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, backlight[index]);

    LightMeterConfig* config = app->config;
    if(index != config->backlight) {
        if(index == BACKLIGHT_ON) {
            notification_message(
                app->notifications,
                &sequence_display_backlight_enforce_on); // force on backlight
        } else {
            notification_message(
                app->notifications,
                &sequence_display_backlight_enforce_auto); // force auto backlight
        }
    }
    config->backlight = index;
    lightmeter_app_set_config(app, config);
}

static void lux_only_cb(VariableItem* item) {
    LightMeterApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, lux_only[index]);

    LightMeterConfig* config = app->config;
    config->lux_only = index;
    lightmeter_app_set_config(app, config);
}

static void sensor_type_cb(VariableItem* item) {
    LightMeterApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, sensor_type[index]);

    LightMeterConfig* config = app->config;
    config->sensor_type = index;
    lightmeter_app_set_config(app, config);
}

static void ok_cb(void* context, uint32_t index) {
    LightMeterApp* app = context;
    UNUSED(app);
    switch(index) {
    case LightMeterSubmenuIndexHelp:
        view_dispatcher_send_custom_event(app->view_dispatcher, LightMeterAppCustomEventHelp);
        break;
    case LightMeterSubmenuIndexAbout:
        view_dispatcher_send_custom_event(app->view_dispatcher, LightMeterAppCustomEventAbout);
        break;
    default:
        break;
    }
}

void lightmeter_scene_config_on_enter(void* context) {
    LightMeterApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;
    VariableItem* item;
    LightMeterConfig* config = app->config;

    item =
        variable_item_list_add(var_item_list, "ISO", COUNT_OF(iso_numbers), iso_numbers_cb, app);
    variable_item_set_current_value_index(item, config->iso);
    variable_item_set_current_value_text(item, iso_numbers[config->iso]);

    item = variable_item_list_add(
        var_item_list, "ND factor", COUNT_OF(nd_numbers), nd_numbers_cb, app);
    variable_item_set_current_value_index(item, config->nd);
    variable_item_set_current_value_text(item, nd_numbers[config->nd]);

    item = variable_item_list_add(
        var_item_list, "Diffusion dome", COUNT_OF(diffusion_dome), dome_presence_cb, app);
    variable_item_set_current_value_index(item, config->dome);
    variable_item_set_current_value_text(item, diffusion_dome[config->dome]);

    item =
        variable_item_list_add(var_item_list, "Backlight", COUNT_OF(backlight), backlight_cb, app);
    variable_item_set_current_value_index(item, config->backlight);
    variable_item_set_current_value_text(item, backlight[config->backlight]);

    item = variable_item_list_add(
        var_item_list, "Lux meter only", COUNT_OF(lux_only), lux_only_cb, app);
    variable_item_set_current_value_index(item, config->lux_only);
    variable_item_set_current_value_text(item, lux_only[config->lux_only]);

    item = variable_item_list_add(
        var_item_list, "Sensor", COUNT_OF(sensor_type), sensor_type_cb, app);
    variable_item_set_current_value_index(item, config->sensor_type);
    variable_item_set_current_value_text(item, sensor_type[config->sensor_type]);

    item = variable_item_list_add(var_item_list, "Help and Pinout", 0, NULL, NULL);
    item = variable_item_list_add(var_item_list, "About", 0, NULL, NULL);

    variable_item_list_set_selected_item(
        var_item_list,
        scene_manager_get_scene_state(app->scene_manager, LightMeterAppSceneConfig));

    variable_item_list_set_enter_callback(var_item_list, ok_cb, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, LightMeterAppViewVarItemList);
}

bool lightmeter_scene_config_on_event(void* context, SceneManagerEvent event) {
    LightMeterApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case LightMeterAppCustomEventHelp:
            scene_manager_next_scene(app->scene_manager, LightMeterAppSceneHelp);
            consumed = true;
            break;
        case LightMeterAppCustomEventAbout:
            scene_manager_next_scene(app->scene_manager, LightMeterAppSceneAbout);
            consumed = true;
            break;
        }
    }
    return consumed;
}

void lightmeter_scene_config_on_exit(void* context) {
    LightMeterApp* app = context;
    variable_item_list_reset(app->var_item_list);
    main_view_set_iso(app->main_view, app->config->iso);
    main_view_set_nd(app->main_view, app->config->nd);
    main_view_set_dome(app->main_view, app->config->dome);
    main_view_set_lux_only(app->main_view, app->config->lux_only);
}
