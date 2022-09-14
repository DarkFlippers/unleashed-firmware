#pragma once

#include <furi.h>
#include <bt/bt_service/bt.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <notification/notification.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include "views/bt_hid_keynote.h"
#include "views/bt_hid_keyboard.h"
#include "views/bt_hid_media.h"
#include "views/bt_hid_mouse.h"

typedef struct {
    Bt* bt;
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    DialogEx* dialog;
    BtHidKeynote* bt_hid_keynote;
    BtHidKeyboard* bt_hid_keyboard;
    BtHidMedia* bt_hid_media;
    BtHidMouse* bt_hid_mouse;
    uint32_t view_id;
} BtHid;

typedef enum {
    BtHidViewSubmenu,
    BtHidViewKeynote,
    BtHidViewKeyboard,
    BtHidViewMedia,
    BtHidViewMouse,
    BtHidViewExitConfirm,
} BtHidView;
