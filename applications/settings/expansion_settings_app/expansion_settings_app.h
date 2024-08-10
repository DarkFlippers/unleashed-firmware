#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/variable_item_list.h>

#include <expansion/expansion.h>
#include <expansion/expansion_settings.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    VariableItemList* var_item_list;
    Expansion* expansion;
    ExpansionSettings settings;
} ExpansionSettingsApp;

typedef enum {
    ExpansionSettingsViewVarItemList,
} ExpansionSettingsView;
