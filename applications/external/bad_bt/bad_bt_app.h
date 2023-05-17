#pragma once

#include "scenes/bad_bt_scene.h"
#include "helpers/ducky_script.h"

#include <gui/gui.h>
#include <assets_icons.h>
#include <gui/scene_manager.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include "bad_bt_icons.h"

#define BAD_BT_APP_BASE_FOLDER EXT_PATH("badusb")
#define BAD_BT_APP_BASE_CONFIG_FOLDER EXT_PATH("badbt")
#define BAD_BT_APP_PATH_LAYOUT_FOLDER BAD_BT_APP_BASE_FOLDER "/assets/layouts"
#define BAD_BT_APP_SCRIPT_EXTENSION ".txt"
#define BAD_BT_APP_LAYOUT_EXTENSION ".kl"

typedef enum BadBtCustomEvent {
    BadBtAppCustomEventTextEditResult,
    BadBtAppCustomEventByteInputDone,
    BadBtCustomEventErrorBack
} BadBtCustomEvent;

typedef enum {
    BadBtAppViewError,
    BadBtAppViewWork,
    BadBtAppViewConfig,
    BadBtAppViewConfigMac,
    BadBtAppViewConfigName
} BadBtAppView;

void bad_bt_config_switch_remember_mode(BadBtApp* app);

int32_t bad_bt_connection_init(BadBtApp* app);

void bad_bt_connection_deinit(BadBtApp* app);

void bad_kb_config_refresh_menu(BadBtApp* app);