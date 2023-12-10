#include "../infrared_app_i.h"

typedef enum {
    SubmenuIndexAddButton,
    SubmenuIndexRenameButton,
    SubmenuIndexMoveButton,
    SubmenuIndexDeleteButton,
    SubmenuIndexRenameRemote,
    SubmenuIndexDeleteRemote,
} SubmenuIndex;

static void infrared_scene_edit_submenu_callback(void* context, uint32_t index) {
    InfraredApp* infrared = context;
    view_dispatcher_send_custom_event(infrared->view_dispatcher, index);
}

void infrared_scene_edit_on_enter(void* context) {
    InfraredApp* infrared = context;
    Submenu* submenu = infrared->submenu;
    SceneManager* scene_manager = infrared->scene_manager;

    submenu_add_item(
        submenu,
        "Add Button",
        SubmenuIndexAddButton,
        infrared_scene_edit_submenu_callback,
        context);
    submenu_add_item(
        submenu,
        "Rename Button",
        SubmenuIndexRenameButton,
        infrared_scene_edit_submenu_callback,
        context);
    submenu_add_item(
        submenu,
        "Move Button",
        SubmenuIndexMoveButton,
        infrared_scene_edit_submenu_callback,
        context);
    submenu_add_item(
        submenu,
        "Delete Button",
        SubmenuIndexDeleteButton,
        infrared_scene_edit_submenu_callback,
        context);
    submenu_add_item(
        submenu,
        "Rename Remote",
        SubmenuIndexRenameRemote,
        infrared_scene_edit_submenu_callback,
        context);
    submenu_add_item(
        submenu,
        "Delete Remote",
        SubmenuIndexDeleteRemote,
        infrared_scene_edit_submenu_callback,
        context);

    const uint32_t submenu_index = scene_manager_get_scene_state(scene_manager, InfraredSceneEdit);
    submenu_set_selected_item(submenu, submenu_index);
    scene_manager_set_scene_state(scene_manager, InfraredSceneEdit, SubmenuIndexAddButton);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewSubmenu);
}

bool infrared_scene_edit_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const uint32_t submenu_index = event.event;
        scene_manager_set_scene_state(scene_manager, InfraredSceneEdit, submenu_index);

        if(submenu_index == SubmenuIndexAddButton) {
            infrared->app_state.is_learning_new_remote = false;
            scene_manager_next_scene(scene_manager, InfraredSceneLearn);
            consumed = true;
        } else if(submenu_index == SubmenuIndexRenameButton) {
            infrared->app_state.edit_target = InfraredEditTargetButton;
            infrared->app_state.edit_mode = InfraredEditModeRename;
            scene_manager_next_scene(scene_manager, InfraredSceneEditButtonSelect);
            consumed = true;
        } else if(submenu_index == SubmenuIndexMoveButton) {
            scene_manager_next_scene(scene_manager, InfraredSceneEditMove);
            consumed = true;
        } else if(submenu_index == SubmenuIndexDeleteButton) {
            infrared->app_state.edit_target = InfraredEditTargetButton;
            infrared->app_state.edit_mode = InfraredEditModeDelete;
            scene_manager_next_scene(scene_manager, InfraredSceneEditButtonSelect);
            consumed = true;
        } else if(submenu_index == SubmenuIndexRenameRemote) {
            infrared->app_state.edit_target = InfraredEditTargetRemote;
            infrared->app_state.edit_mode = InfraredEditModeRename;
            scene_manager_next_scene(scene_manager, InfraredSceneEditRename);
            consumed = true;
        } else if(submenu_index == SubmenuIndexDeleteRemote) {
            infrared->app_state.edit_target = InfraredEditTargetRemote;
            infrared->app_state.edit_mode = InfraredEditModeDelete;
            scene_manager_next_scene(scene_manager, InfraredSceneEditDelete);
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_edit_on_exit(void* context) {
    InfraredApp* infrared = context;
    submenu_reset(infrared->submenu);
}
