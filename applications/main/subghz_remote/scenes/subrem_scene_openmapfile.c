#include "../subghz_remote_app_i.h"

void subrem_scene_openmapfile_on_enter(void* context) {
    SubGhzRemoteApp* app = context;
    SubRemLoadMapState load_state = subrem_load_from_file(app);

    if(load_state == SubRemLoadMapStateError) {
#ifdef SUBREM_LIGHT
        dialog_message_show_storage_error(app->dialogs, "Can't load\nMap file");
#else
        DialogMessage* message = dialog_message_alloc();

        dialog_message_set_header(message, "Map File Error", 64, 8, AlignCenter, AlignCenter);
        dialog_message_set_text(message, "Can't load\nMap file", 64, 32, AlignCenter, AlignCenter);
        dialog_message_set_buttons(message, "Back", NULL, NULL);
        dialog_message_show(app->dialogs, message);

        dialog_message_free(message);
#endif
    }
    if(load_state == SubRemLoadMapStateOK || load_state == SubRemLoadMapStateNotAllOK) {
        scene_manager_next_scene(app->scene_manager, SubRemSceneRemote);
    } else {
        // TODO: Map Preset Reset
        if(!scene_manager_search_and_switch_to_previous_scene(
               app->scene_manager, SubRemSceneStart)) {
            scene_manager_stop(app->scene_manager);
            view_dispatcher_stop(app->view_dispatcher);
        }
    }
}

bool subrem_scene_openmapfile_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void subrem_scene_openmapfile_on_exit(void* context) {
    UNUSED(context);
}
