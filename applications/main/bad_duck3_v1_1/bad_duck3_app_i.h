#pragma once

#include "bad_duck3_app.h"
#include "scenes/bad_duck3_scene.h"
#include "helpers/duck3_script.h"
#include "helpers/duck3_hid.h"

#include <gui/gui.h>
#include <assets_icons.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/widget.h>
#include <gui/modules/popup.h>
#include "views/bad_duck3_view.h"
#include <furi_hal_usb.h>

#define BAD_DUCK3_APP_BASE_FOLDER        EXT_PATH("badusb")
#define BAD_DUCK3_APP_PATH_LAYOUT_FOLDER BAD_DUCK3_APP_BASE_FOLDER "/assets/layouts"
#define BAD_DUCK3_APP_SCRIPT_EXTENSION   ".txt"
#define BAD_DUCK3_APP_LAYOUT_EXTENSION   ".kl"

typedef enum {
    BadDuck3AppErrorNoFiles,
    BadDuck3AppErrorCloseRpc,
} BadDuck3AppError;

struct BadDuck3App {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    DialogsApp* dialogs;
    Widget* widget;
    Popup* popup;
    VariableItemList* var_item_list;
    TextInput* text_input;
    ByteInput* byte_input;

    char ble_name_buf[FURI_HAL_BT_ADV_NAME_LENGTH];
    uint8_t ble_mac_buf[GAP_MAC_ADDR_SIZE];
    char usb_name_buf[HID_MANUF_PRODUCT_NAME_LEN];
    uint16_t usb_vidpid_buf[2];

    BadDuck3AppError error;
    FuriString* file_path;
    FuriString* keyboard_layout;
    BadDuck3View* bad_duck3_view;
    Duck3Script* duck3_script;

    Duck3HidInterface interface;
    Duck3HidConfig user_hid_cfg;
    Duck3HidConfig script_hid_cfg;
    FuriHalUsbInterface* usb_if_prev;
};

typedef enum {
    BadDuck3AppViewWidget,
    BadDuck3AppViewPopup,
    BadDuck3AppViewWork,
    BadDuck3AppViewConfig,
    BadDuck3AppViewByteInput,
    BadDuck3AppViewTextInput,
} BadDuck3AppView;

void bad_duck3_set_interface(BadDuck3App* app, Duck3HidInterface interface);
