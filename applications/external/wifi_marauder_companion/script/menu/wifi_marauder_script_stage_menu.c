#include "wifi_marauder_script_stage_menu.h"

WifiMarauderScriptStageMenu*
    wifi_marauder_script_stage_menu_create(WifiMarauderScriptStageType stage_type) {
    WifiMarauderScriptStageMenu* script_stage_menu = malloc(sizeof(WifiMarauderScriptStageMenu));

    switch(stage_type) {
#define ADD_STAGE(name, id)                                               \
    case WifiMarauderScriptStageType##id:                                 \
        wifi_marauder_script_stage_menu_##name##_load(script_stage_menu); \
        break;

#include "wifi_marauder_script_stage_menu_config.h"
#undef ADD_STAGE
    }
    return script_stage_menu;
}

void wifi_marauder_script_stage_menu_free(WifiMarauderScriptStageMenu* stage_menu) {
    if(stage_menu == NULL) {
        return;
    }
    for(uint32_t i = 0; i < stage_menu->num_items; i++) {
        WifiMarauderScriptMenuItem* item = &(stage_menu->items[i]);
        for(int j = 0; j < item->num_options; j++) {
            free(item->options[j]);
        }
        free(item->name);
    }
    free(stage_menu->items);
    free(stage_menu);
}
