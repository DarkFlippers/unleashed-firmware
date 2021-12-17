#pragma once

#include <furi.h>
#include <bt/bt_service/bt.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <applications/notification/notification.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include "views/bt_hid_keynote.h"
#include "views/bt_hid_media.h"

typedef struct {
    Bt* bt;
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    DialogEx* dialog;
    BtHidKeynote* bt_hid_keynote;
    BtHidMedia* bt_hid_media;
    uint32_t view_id;
} BtHid;

typedef enum {
    BtHidViewSubmenu,
    BtHidViewKeynote,
    BtHidViewMedia,
    BtHidViewExitConfirm,
} BtHidView;
