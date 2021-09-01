#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>

#include "../bt_settings.h"
#include "scenes/bt_settings_scene.h"

typedef struct {
    BtSettings settings;
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    DialogEx* dialog_ex;
} BtSettingsApp;

typedef enum {
    BtSettingsAppViewSubmenu,
    BtSettingsAppViewDialogEx,
} BtSettingsAppView;
