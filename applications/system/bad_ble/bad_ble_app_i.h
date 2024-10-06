#pragma once

#include "bad_ble_app.h"
#include "scenes/bad_ble_scene.h"
#include "helpers/ducky_script.h"
#include "helpers/bad_ble_hid.h"

#include <gui/gui.h>
#include <assets_icons.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include <gui/modules/popup.h>
#include "views/bad_ble_view.h"

#define BAD_BLE_APP_BASE_FOLDER        EXT_PATH("badusb")
#define BAD_BLE_APP_PATH_LAYOUT_FOLDER BAD_BLE_APP_BASE_FOLDER "/assets/layouts"
#define BAD_BLE_APP_SCRIPT_EXTENSION   ".txt"
#define BAD_BLE_APP_LAYOUT_EXTENSION   ".kl"

typedef enum {
    BadBleAppErrorNoFiles,
    BadBleAppErrorCloseRpc,
} BadBleAppError;

struct BadBleApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    DialogsApp* dialogs;
    Widget* widget;
    Popup* popup;
    VariableItemList* var_item_list;

    BadBleAppError error;
    FuriString* file_path;
    FuriString* keyboard_layout;
    BadBle* bad_ble_view;
    BadBleScript* bad_ble_script;

    BadBleHidInterface interface;
};

typedef enum {
    BadBleAppViewWidget,
    BadBleAppViewPopup,
    BadBleAppViewWork,
    BadBleAppViewConfig,
} BadBleAppView;
