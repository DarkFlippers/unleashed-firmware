#pragma once

#include "helpers/weather_station_types.h"

#include "scenes/weather_station_scene.h"
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include <notification/notification_messages.h>
#include "views/weather_station_receiver.h"
#include "views/weather_station_receiver_info.h"
#include "weather_station_history.h"

#include <lib/subghz/subghz_setting.h>
#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/registry.h>

#include "helpers/radio_device_loader.h"

typedef struct WeatherStationApp WeatherStationApp;

struct WeatherStationTxRx {
    SubGhzWorker* worker;

    const SubGhzDevice* radio_device;
    SubGhzEnvironment* environment;
    SubGhzReceiver* receiver;
    SubGhzRadioPreset* preset;
    WSHistory* history;
    uint16_t idx_menu_chosen;
    WSTxRxState txrx_state;
    WSHopperState hopper_state;
    uint8_t hopper_timeout;
    uint8_t hopper_idx_frequency;
    WSRxKeyState rx_key_state;
};

typedef struct WeatherStationTxRx WeatherStationTxRx;

struct WeatherStationApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    WeatherStationTxRx* txrx;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    VariableItemList* variable_item_list;
    Submenu* submenu;
    Widget* widget;
    WSReceiver* ws_receiver;
    WSReceiverInfo* ws_receiver_info;
    WSLock lock;
    SubGhzSetting* setting;
};

void ws_preset_init(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint8_t* preset_data,
    size_t preset_data_size);
bool ws_set_preset(WeatherStationApp* app, const char* preset);
void ws_get_frequency_modulation(
    WeatherStationApp* app,
    FuriString* frequency,
    FuriString* modulation);
void ws_begin(WeatherStationApp* app, uint8_t* preset_data);
uint32_t ws_rx(WeatherStationApp* app, uint32_t frequency);
void ws_idle(WeatherStationApp* app);
void ws_rx_end(WeatherStationApp* app);
void ws_sleep(WeatherStationApp* app);
void ws_hopper_update(WeatherStationApp* app);
