#pragma once

#include "bad_usb_app.h"
#include "scenes/bad_usb_scene.h"
#include "bad_usb_script.h"

#include <gui/gui.h>
#include <assets_icons.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include "views/bad_usb_view.h"

#define BAD_USB_APP_BASE_FOLDER ANY_PATH("badusb")
#define BAD_USB_APP_PATH_LAYOUT_FOLDER BAD_USB_APP_BASE_FOLDER "/assets/layouts"
#define BAD_USB_APP_SCRIPT_EXTENSION ".txt"
#define BAD_USB_APP_LAYOUT_EXTENSION ".kl"

typedef enum {
    BadUsbAppErrorNoFiles,
    BadUsbAppErrorCloseRpc,
} BadUsbAppError;

struct BadUsbApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    DialogsApp* dialogs;
    Widget* widget;
    Submenu* submenu;

    BadUsbAppError error;
    FuriString* file_path;
    FuriString* keyboard_layout;
    BadUsb* bad_usb_view;
    BadUsbScript* bad_usb_script;
};

typedef enum {
    BadUsbAppViewError,
    BadUsbAppViewWork,
    BadUsbAppViewConfig,
} BadUsbAppView;