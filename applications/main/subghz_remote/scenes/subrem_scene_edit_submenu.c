#include "../subghz_remote_app_i.h"
#include "../helpers/subrem_custom_event.h"

void subrem_scene_edit_submenu_text_input_callback(void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SubRemCustomEventSceneEditsubmenu);
}

void subrem_scene_edit_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void subrem_scene_edit_submenu_on_enter(void* context) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    Submenu* submenu = app->submenu;
    submenu_add_item(
        submenu, "Edit Label", EditSubmenuIndexEditLabel, subrem_scene_edit_submenu_callback, app);
    submenu_add_item(
        submenu, "Edit File", EditSubmenuIndexEditFile, subrem_scene_edit_submenu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDSubmenu);
}

bool subrem_scene_edit_submenu_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == EditSubmenuIndexEditLabel) {
            scene_manager_next_scene(app->scene_manager, SubRemSceneEditLabel);
            consumed = true;
        } else if(event.event == EditSubmenuIndexEditFile) {
            scene_manager_next_scene(app->scene_manager, SubRemSceneOpenSubFile);
            consumed = true;
        }
    }

    return consumed;
}

void subrem_scene_edit_submenu_on_exit(void* context) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    submenu_reset(app->submenu);
}
