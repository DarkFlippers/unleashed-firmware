#include "../lfrfid_i.h"

typedef enum {
    SubmenuIndexWriteTargets,
} SubmenuIndex;

static void lfrfid_scene_settings_submenu_callback(void* context, uint32_t index) {
    LfRfid* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void lfrfid_scene_settings_on_enter(void* context) {
    LfRfid* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Write Chips",
        SubmenuIndexWriteTargets,
        lfrfid_scene_settings_submenu_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, LfRfidSceneSettings));

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewSubmenu);
}

bool lfrfid_scene_settings_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexWriteTargets) {
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSettingsWriteTargets);
            consumed = true;
        }
        scene_manager_set_scene_state(app->scene_manager, LfRfidSceneSettings, event.event);

    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_set_scene_state(app->scene_manager, LfRfidSceneSettings, 0);
    }

    return consumed;
}

void lfrfid_scene_settings_on_exit(void* context) {
    LfRfid* app = context;

    submenu_reset(app->submenu);
}
