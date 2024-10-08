#include "../bad_ble_app_i.h"

enum SubmenuIndex {
    ConfigIndexKeyboardLayout,
    ConfigIndexBleUnpair,
};

void bad_ble_scene_config_select_callback(void* context, uint32_t index) {
    BadBleApp* bad_ble = context;

    view_dispatcher_send_custom_event(bad_ble->view_dispatcher, index);
}

static void draw_menu(BadBleApp* bad_ble) {
    VariableItemList* var_item_list = bad_ble->var_item_list;

    variable_item_list_reset(var_item_list);

    variable_item_list_add(var_item_list, "Keyboard Layout (Global)", 0, NULL, NULL);

    variable_item_list_add(var_item_list, "Unpair Device", 0, NULL, NULL);
}

void bad_ble_scene_config_on_enter(void* context) {
    BadBleApp* bad_ble = context;
    VariableItemList* var_item_list = bad_ble->var_item_list;

    variable_item_list_set_enter_callback(
        var_item_list, bad_ble_scene_config_select_callback, bad_ble);
    draw_menu(bad_ble);
    variable_item_list_set_selected_item(var_item_list, 0);

    view_dispatcher_switch_to_view(bad_ble->view_dispatcher, BadBleAppViewConfig);
}

bool bad_ble_scene_config_on_event(void* context, SceneManagerEvent event) {
    BadBleApp* bad_ble = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == ConfigIndexKeyboardLayout) {
            scene_manager_next_scene(bad_ble->scene_manager, BadBleSceneConfigLayout);
        } else if(event.event == ConfigIndexBleUnpair) {
            scene_manager_next_scene(bad_ble->scene_manager, BadBleSceneConfirmUnpair);
        } else {
            furi_crash("Unknown key type");
        }
    }

    return consumed;
}

void bad_ble_scene_config_on_exit(void* context) {
    BadBleApp* bad_ble = context;
    VariableItemList* var_item_list = bad_ble->var_item_list;

    variable_item_list_reset(var_item_list);
}
