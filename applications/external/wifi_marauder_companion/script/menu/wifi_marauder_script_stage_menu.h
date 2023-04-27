#pragma once

#include <gui/modules/variable_item_list.h>
#include "../wifi_marauder_script.h"

#define ITEM_EDIT_MAX_OPTIONS (12)

typedef void (*VariableItemSetupCallback)(VariableItem* item);
typedef void (*VariableItemSelectCallback)(void* context);

typedef enum WifiMarauderScriptMenuItemType {
    WifiMarauderScriptMenuItemTypeString,
    WifiMarauderScriptMenuItemTypeNumber,
    WifiMarauderScriptMenuItemTypeOptionsString,
    WifiMarauderScriptMenuItemTypeOptionsNumber,
    WifiMarauderScriptMenuItemTypeListString,
    WifiMarauderScriptMenuItemTypeListNumber
} WifiMarauderScriptMenuItemType;

typedef struct WifiMarauderScriptMenuItem {
    char* name;
    WifiMarauderScriptMenuItemType type;
    int num_options;
    char* options[ITEM_EDIT_MAX_OPTIONS];
    VariableItemSetupCallback setup_callback;
    VariableItemChangeCallback change_callback;
    VariableItemSelectCallback select_callback;
} WifiMarauderScriptMenuItem;

typedef struct WifiMarauderScriptStageMenu {
    WifiMarauderScriptMenuItem* items;
    uint32_t num_items;
} WifiMarauderScriptStageMenu;

#define ADD_STAGE(name, id) \
    void wifi_marauder_script_stage_menu_##name##_load(WifiMarauderScriptStageMenu*);
#include "wifi_marauder_script_stage_menu_config.h"
#undef ADD_STAGE

WifiMarauderScriptStageMenu*
    wifi_marauder_script_stage_menu_create(WifiMarauderScriptStageType stage_type);
void wifi_marauder_script_stage_menu_free(WifiMarauderScriptStageMenu* list);
