#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <stream/stream.h>
#include <flipper_format/flipper_format_i.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#include "gui/views/main_view.h"

#include <gui/modules/widget.h>
#include <gui/modules/variable_item_list.h>

#include "gui/scenes/config/lightmeter_scene.h"
#include <notification/notification_messages.h>

#include "lightmeter_config.h"
#include <BH1750.h>
#include <MAX44009.h>

#define APP_PATH_DIR STORAGE_APP_DATA_PATH_PREFIX
#define APP_PATH_CFG "config.txt"

typedef struct {
    int32_t iso;
    int32_t nd;
    int32_t aperture;
    int32_t dome;
    int32_t backlight;
    int32_t lux_only;
    int32_t sensor_type;
    int32_t measurement_resolution;
    int32_t device_addr;
} LightMeterConfig;

typedef struct {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    MainView* main_view;
    VariableItemList* var_item_list;
    VariableItem* var_item_addr;
    LightMeterConfig* config;
    NotificationApp* notifications;
    Widget* widget;

    Storage* storage;
    FuriString* cfg_path;
} LightMeterApp;

typedef enum {
    LightMeterAppViewMainView,
    LightMeterAppViewConfigView,
    LightMeterAppViewVarItemList,
    LightMeterAppViewAbout,
    LightMeterAppViewHelp,
} LightMeterAppView;

typedef enum {
    LightMeterAppCustomEventReset,
    LightMeterAppCustomEventConfig,
    LightMeterAppCustomEventHelp,
    LightMeterAppCustomEventAbout,
} LightMeterAppCustomEvent;

void lightmeter_app_set_config(LightMeterApp* context, LightMeterConfig* config);

void lightmeter_app_i2c_init_sensor(LightMeterApp* context);

void lightmeter_app_i2c_deinit_sensor(LightMeterApp* context);

void lightmeter_app_i2c_callback(LightMeterApp* context);

void lightmeter_app_reset_callback(LightMeterApp* context);
