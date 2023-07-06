#pragma once

#include "helpers/subghz_test_types.h"
#include "helpers/subghz_test_event.h"

#include "scenes/subghz_test_scene.h"
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/modules/popup.h>
#include <notification/notification_messages.h>

#include "views/subghz_test_static.h"
#include "views/subghz_test_carrier.h"
#include "views/subghz_test_packet.h"

typedef struct SubGhzTestApp SubGhzTestApp;

struct SubGhzTestApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    Submenu* submenu;
    Widget* widget;
    Popup* popup;
    SubGhzTestStatic* subghz_test_static;
    SubGhzTestCarrier* subghz_test_carrier;
    SubGhzTestPacket* subghz_test_packet;
};
