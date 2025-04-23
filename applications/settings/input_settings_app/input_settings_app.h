#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/variable_item_list.h>
#include <input/input.h>
#include <lib/toolbox/value_index.h>
#include <furi_hal_vibro.h>
#include <storage/storage.h>

// input_settings_app stucture
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    VariableItemList* variable_item_list;
    InputSettings* settings;
} InputSettingsApp;

// list of menu views for view dispatcher
typedef enum {
    InputSettingsViewVariableItemList,
} InputSettingsView;
