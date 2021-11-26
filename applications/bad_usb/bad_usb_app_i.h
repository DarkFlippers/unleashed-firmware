#pragma once

#include "bad_usb_app.h"
#include "scenes/bad_usb_scene.h"
#include "bad_usb_script.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <dialogs/dialogs.h>
#include <notification/notification-messages.h>
#include <gui/modules/variable-item-list.h>
#include "views/bad_usb_view.h"

#define BAD_USB_APP_PATH_FOLDER "/any/badusb"
#define BAD_USB_APP_EXTENSION ".txt"
#define BAD_USB_FILE_NAME_LEN 40

struct BadUsbApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    DialogsApp* dialogs;

    char file_name[BAD_USB_FILE_NAME_LEN + 1];
    BadUsb* bad_usb_view;
    BadUsbScript* bad_usb_script;
};

typedef enum {
    BadUsbAppViewFileSelect,
    BadUsbAppViewWork,
} BadUsbAppView;
