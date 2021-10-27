#include "../subghz_i.h"

enum SubmenuIndex {
    SubmenuIndexEmulate,
    SubmenuIndexEdit,
    SubmenuIndexDelete,
};

void subghz_scene_read_raw_menu_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

void subghz_scene_read_raw_menu_on_enter(void* context) {
    SubGhz* subghz = context;
    submenu_add_item(
        subghz->submenu,
        "Emulate",
        SubmenuIndexEmulate,
        subghz_scene_read_raw_menu_submenu_callback,
        subghz);

    submenu_add_item(
        subghz->submenu,
        "Save",
        SubmenuIndexEdit,
        subghz_scene_read_raw_menu_submenu_callback,
        subghz);

    submenu_add_item(
        subghz->submenu,
        "Delete",
        SubmenuIndexDelete,
        subghz_scene_read_raw_menu_submenu_callback,
        subghz);

    submenu_set_selected_item(
        subghz->submenu,
        scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneSavedMenu));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewMenu);
}

bool subghz_scene_read_raw_menu_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexEmulate) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneReadRAWMenu, SubmenuIndexEmulate);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTransmitter);
            return true;
        } else if(event.event == SubmenuIndexDelete) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneReadRAWMenu, SubmenuIndexDelete);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneDelete);
            return true;
        } else if(event.event == SubmenuIndexEdit) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneReadRAWMenu, SubghzCustomEventManagerSet);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            return true;
        }
    }
    return false;
}

void subghz_scene_read_raw_menu_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_clean(subghz->submenu);
    subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
}
