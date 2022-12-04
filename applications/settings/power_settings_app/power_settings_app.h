#pragma once

#include <furi.h>
#include <power/power_service/power.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <assets_icons.h>

#include "views/battery_info.h"
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/variable_item_list.h>

#include <power/power_settings.h>
#include "scenes/power_settings_scene.h"

typedef struct {
    Power* power;
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    BatteryInfo* batery_info;
    Submenu* submenu;
    DialogEx* dialog;
    PowerInfo info;
    VariableItemList* variable_item_list;
    uint32_t shutdown_idle_delay_ms;
    FuriPubSub* settings_events;
} PowerSettingsApp;

typedef enum {
    PowerSettingsAppViewBatteryInfo,
    PowerSettingsAppViewSubmenu,
    PowerSettingsAppViewDialog,
    PowerSettingsAppViewVariableItemList
} PowerSettingsAppView;
