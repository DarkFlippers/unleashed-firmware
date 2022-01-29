#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/code_input.h>

#include "desktop_settings.h"

typedef enum {
    CodeEventsSetPin,
    CodeEventsChangePin,
    CodeEventsDisablePin,
} CodeEventsEnum;

typedef enum {
    DesktopSettingsAppViewMenu,
    DesktopSettingsAppViewPincodeInput,
} DesktopSettingsAppView;

typedef struct {
    DesktopSettings settings;

    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    CodeInput* code_input;

    uint8_t menu_idx;

} DesktopSettingsApp;
