#include "../infrared_app_i.h"

void infrared_scene_learn_done_on_enter(void* context) {
    InfraredApp* infrared = context;
    Popup* popup = infrared->popup;

    if(infrared->app_state.is_learning_new_remote) {
        popup_set_icon(popup, 48, 6, &I_DolphinDone_80x58);
        popup_set_header(popup, "New remote\ncreated!", 0, 0, AlignLeft, AlignTop);
    } else {
        popup_set_icon(popup, 36, 5, &I_DolphinSaved_92x58);
        popup_set_header(popup, "Saved", 15, 19, AlignLeft, AlignBottom);
    }

    popup_set_callback(popup, infrared_popup_closed_callback);
    popup_set_context(popup, context);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);
}

bool infrared_scene_learn_done_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypePopupClosed) {
            if(!scene_manager_search_and_switch_to_previous_scene(
                   infrared->scene_manager, InfraredSceneRemote)) {
                scene_manager_next_scene(infrared->scene_manager, InfraredSceneRemote);
            }
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_learn_done_on_exit(void* context) {
    InfraredApp* infrared = context;
    infrared->app_state.is_learning_new_remote = false;
    popup_set_header(infrared->popup, NULL, 0, 0, AlignLeft, AlignTop);
}
