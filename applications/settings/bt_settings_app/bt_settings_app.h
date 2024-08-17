#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#include <gui/modules/variable_item_list.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>

#include <bt/bt_service/bt.h>
#include <bt/bt_service/bt_settings_api_i.h>

#include <assets_icons.h>

#include "scenes/bt_settings_scene.h"

enum BtSettingsCustomEvent {
    // Keep first 10 events reserved for button types and indexes
    BtSettingsCustomEventReserved = 10,

    BtSettingsCustomEventForgetDevices,
    BtSettingsCustomEventExitView,
};

typedef struct {
    BtSettings settings;
    Bt* bt;
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    VariableItemList* var_item_list;
    DialogEx* dialog;
    Popup* popup;
} BtSettingsApp;

typedef enum {
    BtSettingsAppViewVarItemList,
    BtSettingsAppViewDialog,
    BtSettingsAppViewPopup,
} BtSettingsAppView;
