#include "../subghz_i.h"

enum SubmenuIndex {
    SubmenuIndexAnalyze,
    SubmenuIndexRead,
    SubmenuIndexSaved,
    SubmenuIndexStatic,
    SubmenuIndexTest,
};

void subghz_scene_start_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

const void subghz_scene_start_on_enter(void* context) {
    SubGhz* subghz = context;

    submenu_add_item(
        subghz->submenu,
        "Analyze",
        SubmenuIndexAnalyze,
        subghz_scene_start_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu, "Read", SubmenuIndexRead, subghz_scene_start_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu, "Saved", SubmenuIndexSaved, subghz_scene_start_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu, "Static", SubmenuIndexStatic, subghz_scene_start_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu, "Test", SubmenuIndexTest, subghz_scene_start_submenu_callback, subghz);

    submenu_set_selected_item(
        subghz->submenu, scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneStart));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewMenu);
}

const bool subghz_scene_start_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexAnalyze) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexAnalyze);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneAnalyze);
            return true;
        } else if(event.event == SubmenuIndexRead) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexRead);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneRead);
            return true;
        } else if(event.event == SubmenuIndexSaved) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexSaved);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaved);
            return true;
        } else if(event.event == SubmenuIndexStatic) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexStatic);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneStatic);
            return true;
        } else if(event.event == SubmenuIndexTest) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexTest);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTest);
            return true;
        }
    }
    return false;
}

const void subghz_scene_start_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_clean(subghz->submenu);
}
