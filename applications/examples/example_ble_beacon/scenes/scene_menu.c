#include "../ble_beacon_app.h"

enum SubmenuIndex {
    SubmenuIndexSetMac,
    SubmenuIndexSetData,
};

static void ble_beacon_app_scene_menu_submenu_callback(void* context, uint32_t index) {
    BleBeaconApp* ble_beacon = context;
    view_dispatcher_send_custom_event(ble_beacon->view_dispatcher, index);
}

void ble_beacon_app_scene_menu_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;
    Submenu* submenu = ble_beacon->submenu;

    submenu_add_item(
        submenu,
        "Set MAC",
        SubmenuIndexSetMac,
        ble_beacon_app_scene_menu_submenu_callback,
        ble_beacon);
    submenu_add_item(
        submenu,
        "Set Data",
        SubmenuIndexSetData,
        ble_beacon_app_scene_menu_submenu_callback,
        ble_beacon);

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewSubmenu);
}

bool ble_beacon_app_scene_menu_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const uint32_t submenu_index = event.event;
        if(submenu_index == SubmenuIndexSetMac) {
            scene_manager_next_scene(scene_manager, BleBeaconAppSceneInputMacAddress);
            consumed = true;
        } else if(submenu_index == SubmenuIndexSetData) {
            scene_manager_next_scene(scene_manager, BleBeaconAppSceneInputBeaconData);
            consumed = true;
        }
    }

    return consumed;
}

void ble_beacon_app_scene_menu_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;
    submenu_reset(ble_beacon->submenu);
}
