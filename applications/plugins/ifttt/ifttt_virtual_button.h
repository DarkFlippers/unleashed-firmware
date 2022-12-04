#pragma once

#include <furi.h>
#include <power/power_service/power.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include "views/send_view.h"
#include "views/about_view.h"
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>
#include <storage/storage.h>
#include <furi_hal_uart.h>
#include "scenes/virtual_button_scene.h"

#define APP_NAME "[ESP8266] IFTTT Virtual Button"

#define CONF_SSID "wifi_ssid"
#define CONF_PASSWORD "wifi_password"
#define CONF_KEY "webhooks_key"
#define CONF_EVENT "event"
#define CONFIG_FILE_HEADER "IFTTT Virtual Button Config File"
#define CONFIG_FILE_VERSION 1

typedef struct {
    char* save_ssid;
    char* save_password;
    char* save_key;
    char* save_event;
} Settings;

typedef struct {
    Power* power;
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    SendView* sen_view;
    AboutView* abou_view;
    Submenu* submenu;
    DialogEx* dialog;
    PowerInfo info;
    Settings settings;
} VirtualButtonApp;

typedef enum {
    VirtualButtonAppViewSendView,
    VirtualButtonAppViewAboutView,
    VirtualButtonAppViewSubmenu,
    VirtualButtonAppViewDialog,
} VirtualButtonAppView;

Settings save_settings(Settings settings);
Settings* load_settings();