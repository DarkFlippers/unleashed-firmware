#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>

#include <dialogs/dialogs.h>

#include "views/bt_carrier_test.h"
#include "views/bt_packet_test.h"

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    BtCarrierTest* bt_carrier_test;
    BtPacketTest* bt_packet_test;
} BtDebugApp;

typedef enum {
    BtDebugAppViewSubmenu,
    BtDebugAppViewCarrierTest,
    BtDebugAppViewPacketTest,
} BtDebugAppView;
