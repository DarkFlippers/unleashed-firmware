#include "../infrared_app_i.h"

static int32_t infrared_scene_remote_list_task_callback(void* context) {
    InfraredApp* infrared = context;
    const bool success =
        infrared_remote_load(infrared->remote, furi_string_get_cstr(infrared->file_path));
    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypeTaskFinished);
    return success;
}

static void infrared_scene_remote_list_select_and_load(InfraredApp* infrared) {
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, INFRARED_APP_EXTENSION, &I_ir_10px);
    browser_options.base_path = INFRARED_APP_FOLDER;

    const bool file_selected = dialog_file_browser_show(
        infrared->dialogs, infrared->file_path, infrared->file_path, &browser_options);

    if(file_selected) {
        // Load the remote in a separate thread
        infrared_blocking_task_start(infrared, infrared_scene_remote_list_task_callback);

    } else {
        scene_manager_previous_scene(infrared->scene_manager);
    }
}

void infrared_scene_remote_list_on_enter(void* context) {
    InfraredApp* infrared = context;
    infrared_scene_remote_list_select_and_load(infrared);
}

bool infrared_scene_remote_list_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypeTaskFinished) {
            const bool task_success = infrared_blocking_task_finalize(infrared);

            if(task_success) {
                scene_manager_next_scene(infrared->scene_manager, InfraredSceneRemote);
            } else {
                infrared_show_error_message(
                    infrared, "Failed to load\n\"%s\"", furi_string_get_cstr(infrared->file_path));
                infrared_scene_remote_list_select_and_load(infrared);
            }
        }
        consumed = true;
    }

    return consumed;
}

void infrared_scene_remote_list_on_exit(void* context) {
    UNUSED(context);
}
