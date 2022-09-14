#include "../infrared_i.h"

void infrared_scene_remote_list_on_enter(void* context) {
    Infrared* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    ViewDispatcher* view_dispatcher = infrared->view_dispatcher;

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, INFRARED_APP_EXTENSION, &I_ir_10px);

    bool success = dialog_file_browser_show(
        infrared->dialogs, infrared->file_path, infrared->file_path, &browser_options);

    if(success) {
        view_set_orientation(view_stack_get_view(infrared->view_stack), ViewOrientationVertical);
        view_dispatcher_switch_to_view(view_dispatcher, InfraredViewStack);

        infrared_show_loading_popup(infrared, true);
        success = infrared_remote_load(infrared->remote, infrared->file_path);
        infrared_show_loading_popup(infrared, false);
    }

    if(success) {
        scene_manager_next_scene(scene_manager, InfraredSceneRemote);
    } else {
        scene_manager_previous_scene(scene_manager);
    }
}

bool infrared_scene_remote_list_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

void infrared_scene_remote_list_on_exit(void* context) {
    UNUSED(context);
}
