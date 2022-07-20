#pragma once

#include "u2f_app.h"
#include "scenes/u2f_scene.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include "views/u2f_view.h"
#include "u2f_hid.h"
#include "u2f.h"

typedef enum {
    U2fAppErrorNoFiles,
    U2fAppErrorCloseRpc,
} U2fAppError;

typedef enum {
    U2fCustomEventNone,

    U2fCustomEventConnect,
    U2fCustomEventDisconnect,
    U2fCustomEventDataError,

    U2fCustomEventRegister,
    U2fCustomEventAuth,
    U2fCustomEventAuthSuccess,
    U2fCustomEventWink,

    U2fCustomEventTimeout,

    U2fCustomEventConfirm,

    U2fCustomEventErrorBack,

} GpioCustomEvent;

typedef enum {
    U2fAppViewError,
    U2fAppViewMain,
} U2fAppView;

struct U2fApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    Widget* widget;
    FuriTimer* timer;
    U2fHid* u2f_hid;
    U2fView* u2f_view;
    U2fData* u2f_instance;
    GpioCustomEvent event_cur;
    bool u2f_ready;
    U2fAppError error;
};
