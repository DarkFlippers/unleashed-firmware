#pragma once

#include "u2f_app.h"
#include "scenes/u2f_scene.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <dialogs/dialogs.h>
#include <notification/notification-messages.h>
#include <gui/modules/variable-item-list.h>
#include "views/u2f_view.h"
#include "u2f_hid.h"
#include "u2f.h"

typedef enum {
    U2fCustomEventNone,

    U2fCustomEventRegister,
    U2fCustomEventAuth,
    U2fCustomEventWink,

    U2fCustomEventTimeout,

    U2fCustomEventConfirm,

} GpioCustomEvent;

typedef enum {
    U2fAppViewMain,
} U2fAppView;

struct U2fApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    osTimerId_t timer;
    U2fHid* u2f_hid;
    U2fView* u2f_view;
    U2fData* u2f_instance;
    GpioCustomEvent event_cur;
    bool u2f_ready;
};
