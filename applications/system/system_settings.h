#pragma once

#include <furi.h>
#include <furi-hal.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/variable-item-list.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    VariableItemList* var_item_list;
} SystemSettings;

typedef enum {
    SystemSettingsViewVarItemList,
} SystemSettingsView;
