#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#include <gui/modules/variable_item_list.h>

#include "../bt_settings.h"
#include "scenes/bt_settings_scene.h"

typedef struct {
    BtSettings settings;
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    VariableItemList* var_item_list;
} BtSettingsApp;

typedef enum { BtSettingsAppViewVarItemList } BtSettingsAppView;
