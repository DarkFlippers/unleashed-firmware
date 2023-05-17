#include "../bad_bt_app.h"
#include "../helpers/ducky_script.h"
#include "furi_hal_power.h"

enum VarItemListIndex {
    VarItemListIndexKeyboardLayout,
    VarItemListIndexBtRemember,
    VarItemListIndexBtDeviceName,
    VarItemListIndexBtMacAddress,
    VarItemListIndexRandomizeBtMac,
};

void bad_bt_scene_config_bt_remember_callback(VariableItem* item) {
    BadBtApp* bad_bt = variable_item_get_context(item);
    bad_bt->bt_remember = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, bad_bt->bt_remember ? "ON" : "OFF");
    view_dispatcher_send_custom_event(bad_bt->view_dispatcher, VarItemListIndexBtRemember);
}

void bad_bt_scene_config_var_item_list_callback(void* context, uint32_t index) {
    BadBtApp* bad_bt = context;
    view_dispatcher_send_custom_event(bad_bt->view_dispatcher, index);
}

void bad_bt_scene_config_on_enter(void* context) {
    BadBtApp* bad_bt = context;
    VariableItemList* var_item_list = bad_bt->var_item_list;
    VariableItem* item;

    item = variable_item_list_add(var_item_list, "Keyboard layout", 0, NULL, bad_bt);

    item = variable_item_list_add(
        var_item_list, "BT Remember", 2, bad_bt_scene_config_bt_remember_callback, bad_bt);
    variable_item_set_current_value_index(item, bad_bt->bt_remember);
    variable_item_set_current_value_text(item, bad_bt->bt_remember ? "ON" : "OFF");

    item = variable_item_list_add(var_item_list, "BT Device Name", 0, NULL, bad_bt);
    if(bad_bt->bad_bt_script->set_bt_id) {
        variable_item_set_locked(item, true, "Script has\nBT_ID cmd!\nLocked to\nset Name!");
    }

    item = variable_item_list_add(var_item_list, "BT MAC Address", 0, NULL, bad_bt);
    if(bad_bt->bt_remember) {
        variable_item_set_locked(item, true, "Remember\nmust be Off!");
    } else if(bad_bt->bad_bt_script->set_bt_id) {
        variable_item_set_locked(item, true, "Script has\nBT_ID cmd!\nLocked to\nset MAC!");
    }

    item = variable_item_list_add(var_item_list, "Randomize BT MAC", 0, NULL, bad_bt);
    if(bad_bt->bt_remember) {
        variable_item_set_locked(item, true, "Remember\nmust be Off!");
    } else if(bad_bt->bad_bt_script->set_bt_id) {
        variable_item_set_locked(item, true, "Script has\nBT_ID cmd!\nLocked to\nset MAC!");
    }

    variable_item_list_set_enter_callback(
        var_item_list, bad_bt_scene_config_var_item_list_callback, bad_bt);

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(bad_bt->scene_manager, BadBtSceneConfig));

    view_dispatcher_switch_to_view(bad_bt->view_dispatcher, BadBtAppViewConfig);
}

bool bad_bt_scene_config_on_event(void* context, SceneManagerEvent event) {
    BadBtApp* bad_bt = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(bad_bt->scene_manager, BadBtSceneConfig, event.event);
        consumed = true;
        switch(event.event) {
        case VarItemListIndexKeyboardLayout:
            scene_manager_next_scene(bad_bt->scene_manager, BadBtSceneConfigLayout);
            break;
        case VarItemListIndexBtRemember:
            bad_bt_config_switch_remember_mode(bad_bt);
            scene_manager_previous_scene(bad_bt->scene_manager);
            scene_manager_next_scene(bad_bt->scene_manager, BadBtSceneConfig);
            break;
        case VarItemListIndexBtDeviceName:
            scene_manager_next_scene(bad_bt->scene_manager, BadBtSceneConfigName);
            break;
        case VarItemListIndexBtMacAddress:
            scene_manager_next_scene(bad_bt->scene_manager, BadBtSceneConfigMac);
            break;
        case VarItemListIndexRandomizeBtMac:
            furi_hal_random_fill_buf(bad_bt->config.bt_mac, BAD_BT_MAC_ADDRESS_LEN);
            bt_set_profile_mac_address(bad_bt->bt, bad_bt->config.bt_mac);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void bad_bt_scene_config_on_exit(void* context) {
    BadBtApp* bad_bt = context;
    VariableItemList* var_item_list = bad_bt->var_item_list;

    variable_item_list_reset(var_item_list);
}
