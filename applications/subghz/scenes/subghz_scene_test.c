#include "../subghz_i.h"

enum SubmenuIndex {
    SubmenuIndexCarrier,
    SubmenuIndexPacket,
    SubmenuIndexStatic,
};

void subghz_scene_test_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

void subghz_scene_test_on_enter(void* context) {
    SubGhz* subghz = context;

    submenu_add_item(
        subghz->submenu,
        "Carrier",
        SubmenuIndexCarrier,
        subghz_scene_test_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu, "Packet", SubmenuIndexPacket, subghz_scene_test_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu, "Static", SubmenuIndexStatic, subghz_scene_test_submenu_callback, subghz);

    submenu_set_selected_item(
        subghz->submenu, scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneTest));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdMenu);
}

bool subghz_scene_test_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexCarrier) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneTest, SubmenuIndexCarrier);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTestCarrier);
            return true;
        } else if(event.event == SubmenuIndexPacket) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneTest, SubmenuIndexPacket);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTestPacket);
            return true;
        } else if(event.event == SubmenuIndexStatic) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneTest, SubmenuIndexStatic);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTestStatic);
            return true;
        }
    }
    return false;
}

void subghz_scene_test_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_reset(subghz->submenu);
}
