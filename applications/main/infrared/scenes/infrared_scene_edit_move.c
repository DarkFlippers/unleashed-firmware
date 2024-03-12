#include "../infrared_app_i.h"

static int32_t infrared_scene_edit_move_task_callback(void* context) {
    InfraredApp* infrared = context;
    const bool success = infrared_remote_move_signal(
        infrared->remote,
        infrared->app_state.prev_button_index,
        infrared->app_state.current_button_index);
    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypeTaskFinished);

    return success;
}

static void infrared_scene_edit_move_button_callback(
    uint32_t index_old,
    uint32_t index_new,
    void* context) {
    InfraredApp* infrared = context;
    furi_assert(infrared);

    infrared->app_state.prev_button_index = index_old;
    infrared->app_state.current_button_index = index_new;

    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypeButtonSelected);
}

void infrared_scene_edit_move_on_enter(void* context) {
    InfraredApp* infrared = context;
    InfraredRemote* remote = infrared->remote;

    for(size_t i = 0; i < infrared_remote_get_signal_count(remote); ++i) {
        infrared_move_view_add_item(
            infrared->move_view, infrared_remote_get_signal_name(remote, i));
    }

    infrared_move_view_set_callback(
        infrared->move_view, infrared_scene_edit_move_button_callback, infrared);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewMove);
}

bool infrared_scene_edit_move_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypeButtonSelected) {
            // Move the button in a separate thread
            infrared_blocking_task_start(infrared, infrared_scene_edit_move_task_callback);

        } else if(event.event == InfraredCustomEventTypeTaskFinished) {
            const bool task_success = infrared_blocking_task_finalize(infrared);

            if(!task_success) {
                const char* signal_name = infrared_remote_get_signal_name(
                    infrared->remote, infrared->app_state.current_button_index);
                infrared_show_error_message(infrared, "Failed to move\n\"%s\"", signal_name);
                scene_manager_search_and_switch_to_previous_scene(
                    infrared->scene_manager, InfraredSceneRemoteList);
            } else {
                view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewMove);
            }
        }
        consumed = true;
    }

    return consumed;
}

void infrared_scene_edit_move_on_exit(void* context) {
    InfraredApp* infrared = context;
    infrared_move_view_reset(infrared->move_view);
}
