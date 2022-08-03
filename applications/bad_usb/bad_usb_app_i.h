#pragma once

#include "bad_usb_app.h"
#include "scenes/bad_usb_scene.h"
#include "bad_usb_script.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include "views/bad_usb_view.h"

#define BAD_USB_APP_PATH_FOLDER ANY_PATH("badusb")
#define BAD_USB_APP_EXTENSION ".txt"

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

    BadUsbAppError error;
    string_t file_path;
    BadUsb* bad_usb_view;
    BadUsbScript* bad_usb_script;
};

typedef enum {
    BadUsbAppViewError,
    BadUsbAppViewWork,
} BadUsbAppView;
