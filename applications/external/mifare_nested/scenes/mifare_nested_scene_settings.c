#include "../mifare_nested_i.h"
#include <lib/toolbox/value_index.h>

enum MifareNestedSettingsIndex { MifareNestedIndexBlock, MifareNestedIndexHardNested };

#define HARD_NESTED_COUNT 2
const char* const hard_nested_text[HARD_NESTED_COUNT] = {
    "No",
    "Yes",
};

const bool hard_nested_value[HARD_NESTED_COUNT] = {
    false,
    true,
};

static void mifare_nested_scene_settings_set_hard_nested(VariableItem* item) {
    MifareNested* mifare_nested = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, hard_nested_text[index]);
    mifare_nested->settings->only_hardnested = hard_nested_value[index];
}

void mifare_nested_scene_settings_on_enter(void* context) {
    MifareNested* mifare_nested = context;
    VariableItem* item;
    uint8_t value_index;

    item = variable_item_list_add(
        mifare_nested->variable_item_list,
        "Hard Nested only:",
        HARD_NESTED_COUNT,
        mifare_nested_scene_settings_set_hard_nested,
        mifare_nested);

    value_index = value_index_bool(
        mifare_nested->settings->only_hardnested, hard_nested_value, HARD_NESTED_COUNT);

    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, hard_nested_text[value_index]);

    view_dispatcher_switch_to_view(mifare_nested->view_dispatcher, MifareNestedViewVariableList);
}

bool mifare_nested_scene_settings_on_event(void* context, SceneManagerEvent event) {
    MifareNested* mifare_nested = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MifareNestedCustomEventSceneSettingLock) {
            scene_manager_previous_scene(mifare_nested->scene_manager);
            consumed = true;
        }
    }

    return consumed;
}

void mifare_nested_scene_settings_on_exit(void* context) {
    MifareNested* mifare_nested = context;
    variable_item_list_set_selected_item(mifare_nested->variable_item_list, 0);
    variable_item_list_reset(mifare_nested->variable_item_list);
    scene_manager_set_scene_state(mifare_nested->scene_manager, MifareNestedSceneStart, 0);
}
