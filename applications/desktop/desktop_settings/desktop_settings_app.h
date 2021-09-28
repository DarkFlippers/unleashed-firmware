#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>

#include "desktop_settings.h"
#include "scenes/desktop_settings_scene.h"

typedef enum {
    DesktopSettingsAppViewMain,
    DesktopSettingsAppViewFavorite,
} DesktopSettingsAppView;

typedef struct {
    DesktopSettings settings;
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;

} DesktopSettingsApp;
