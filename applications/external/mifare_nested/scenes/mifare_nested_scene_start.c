#include "../mifare_nested_i.h"
enum SubmenuIndex {
    SubmenuIndexCollect,
    SubmenuIndexCheck,
    SubmenuIndexSettings,
    SubmenuIndexAbout
};

void mifare_nested_scene_start_submenu_callback(void* context, uint32_t index) {
    MifareNested* mifare_nested = context;
    view_dispatcher_send_custom_event(mifare_nested->view_dispatcher, index);
}

void mifare_nested_scene_start_on_enter(void* context) {
    MifareNested* mifare_nested = context;

    Submenu* submenu = mifare_nested->submenu;
    submenu_add_item(
        submenu,
        "Nested attack",
        SubmenuIndexCollect,
        mifare_nested_scene_start_submenu_callback,
        mifare_nested);

    submenu_add_item(
        submenu,
        "Check found keys",
        SubmenuIndexCheck,
        mifare_nested_scene_start_submenu_callback,
        mifare_nested);

    submenu_add_item(
        submenu,
        "Settings",
        SubmenuIndexSettings,
        mifare_nested_scene_start_submenu_callback,
        mifare_nested);

    submenu_add_item(
        submenu,
        "About",
        SubmenuIndexAbout,
        mifare_nested_scene_start_submenu_callback,
        mifare_nested);

    submenu_set_selected_item(
        submenu,
        scene_manager_get_scene_state(mifare_nested->scene_manager, MifareNestedSceneStart));
    view_dispatcher_switch_to_view(mifare_nested->view_dispatcher, MifareNestedViewMenu);
}

bool mifare_nested_scene_start_on_event(void* context, SceneManagerEvent event) {
    MifareNested* mifare_nested = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexCollect) {
            mifare_nested->run = NestedRunAttack;
            scene_manager_next_scene(mifare_nested->scene_manager, MifareNestedSceneCheck);
            consumed = true;
        } else if(event.event == SubmenuIndexCheck) {
            mifare_nested->run = NestedRunCheckKeys;
            scene_manager_next_scene(mifare_nested->scene_manager, MifareNestedSceneCheck);
            consumed = true;
        } else if(event.event == SubmenuIndexSettings) {
            mifare_nested->keys->found_keys = 123;
            scene_manager_next_scene(mifare_nested->scene_manager, MifareNestedSceneSettings);
            consumed = true;
        } else if(event.event == SubmenuIndexAbout) {
            scene_manager_next_scene(mifare_nested->scene_manager, MifareNestedSceneAbout);
            consumed = true;
        }

        scene_manager_set_scene_state(
            mifare_nested->scene_manager, MifareNestedSceneStart, event.event);
    }

    return consumed;
}

void mifare_nested_scene_start_on_exit(void* context) {
    MifareNested* mifare_nested = context;
    submenu_reset(mifare_nested->submenu);
}
