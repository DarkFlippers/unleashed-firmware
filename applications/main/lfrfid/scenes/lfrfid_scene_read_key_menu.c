#include "../lfrfid_i.h"
#include <dolphin/dolphin.h>

typedef enum {
    SubmenuIndexSave,
    SubmenuIndexEmulate,
    SubmenuIndexWrite,
} SubmenuIndex;

void lfrfid_scene_read_key_menu_submenu_callback(void* context, uint32_t index) {
    LfRfid* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void lfrfid_scene_read_key_menu_on_enter(void* context) {
    LfRfid* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu, "Save", SubmenuIndexSave, lfrfid_scene_read_key_menu_submenu_callback, app);
    submenu_add_item(
        submenu, "Emulate", SubmenuIndexEmulate, lfrfid_scene_read_key_menu_submenu_callback, app);
    submenu_add_item(
        submenu, "Write", SubmenuIndexWrite, lfrfid_scene_read_key_menu_submenu_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, LfRfidSceneReadKeyMenu));

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewSubmenu);
}

bool lfrfid_scene_read_key_menu_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexWrite) {
            scene_manager_next_scene(app->scene_manager, LfRfidSceneWrite);
            consumed = true;
        } else if(event.event == SubmenuIndexSave) {
            furi_string_reset(app->file_name);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSaveName);
            consumed = true;
        } else if(event.event == SubmenuIndexEmulate) {
            scene_manager_next_scene(app->scene_manager, LfRfidSceneEmulate);
            dolphin_deed(DolphinDeedRfidEmulate);
            consumed = true;
        }
        scene_manager_set_scene_state(app->scene_manager, LfRfidSceneReadKeyMenu, event.event);
    }

    return consumed;
}

void lfrfid_scene_read_key_menu_on_exit(void* context) {
    LfRfid* app = context;

    submenu_reset(app->submenu);
}
