#include "../infrared_app_i.h"

void infrared_scene_remote_list_on_enter(void* context) {
    InfraredApp* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    ViewDispatcher* view_dispatcher = infrared->view_dispatcher;

    view_set_orientation(view_stack_get_view(infrared->view_stack), ViewOrientationVertical);
    view_dispatcher_switch_to_view(view_dispatcher, InfraredViewStack);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, INFRARED_APP_EXTENSION, &I_ir_10px);
    browser_options.base_path = INFRARED_APP_FOLDER;

    while(dialog_file_browser_show(
        infrared->dialogs, infrared->file_path, infrared->file_path, &browser_options)) {
        const char* file_path = furi_string_get_cstr(infrared->file_path);

        infrared_show_loading_popup(infrared, true);
        const bool remote_loaded = infrared_remote_load(infrared->remote, file_path);
        infrared_show_loading_popup(infrared, false);

        if(remote_loaded) {
            scene_manager_next_scene(scene_manager, InfraredSceneRemote);
            return;
        } else {
            infrared_show_error_message(infrared, "Failed to load\n\"%s\"", file_path);
        }
    }

    scene_manager_previous_scene(scene_manager);
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
