#include "../infrared_i.h"

static void infrared_scene_dialog_result_callback(DialogExResult result, void* context) {
    Infrared* infrared = context;
    view_dispatcher_send_custom_event(infrared->view_dispatcher, result);
}

void infrared_scene_ask_back_on_enter(void* context) {
    Infrared* infrared = context;
    DialogEx* dialog_ex = infrared->dialog_ex;

    if(infrared->app_state.is_learning_new_remote) {
        dialog_ex_set_header(dialog_ex, "Exit to Infrared Menu?", 64, 0, AlignCenter, AlignTop);
    } else {
        dialog_ex_set_header(dialog_ex, "Exit to Remote Menu?", 64, 0, AlignCenter, AlignTop);
    }

    dialog_ex_set_text(
        dialog_ex, "All unsaved data\nwill be lost!", 64, 31, AlignCenter, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
    dialog_ex_set_left_button_text(dialog_ex, "Exit");
    dialog_ex_set_center_button_text(dialog_ex, NULL);
    dialog_ex_set_right_button_text(dialog_ex, "Stay");
    dialog_ex_set_result_callback(dialog_ex, infrared_scene_dialog_result_callback);
    dialog_ex_set_context(dialog_ex, context);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewDialogEx);
}

bool infrared_scene_ask_back_on_event(void* context, SceneManagerEvent event) {
    Infrared* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            if(infrared->app_state.is_learning_new_remote) {
                scene_manager_search_and_switch_to_previous_scene(
                    scene_manager, InfraredSceneStart);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    scene_manager, InfraredSceneRemote);
            }
            consumed = true;
        } else if(event.event == DialogExResultRight) {
            scene_manager_previous_scene(scene_manager);
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_ask_back_on_exit(void* context) {
    Infrared* infrared = context;
    dialog_ex_reset(infrared->dialog_ex);
}
