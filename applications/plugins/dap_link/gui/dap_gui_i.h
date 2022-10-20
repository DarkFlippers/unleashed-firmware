#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <notification/notification_messages.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>

#include "dap_gui.h"
#include "../dap_link.h"
#include "scenes/config/dap_scene.h"
#include "dap_gui_custom_event.h"
#include "views/dap_main_view.h"

typedef struct {
    DapApp* dap_app;

    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    VariableItemList* var_item_list;
    DapMainView* main_view;
    Widget* widget;
} DapGuiApp;

typedef enum {
    DapGuiAppViewVarItemList,
    DapGuiAppViewMainView,
    DapGuiAppViewWidget,
} DapGuiAppView;
