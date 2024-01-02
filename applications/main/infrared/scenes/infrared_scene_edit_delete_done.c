#include "../infrared_app_i.h"

void infrared_scene_edit_delete_done_on_enter(void* context) {
    InfraredApp* infrared = context;
    Popup* popup = infrared->popup;

    popup_set_icon(popup, 0, 2, &I_DolphinMafia_119x62);
    popup_set_header(popup, "Deleted", 80, 19, AlignLeft, AlignBottom);
    popup_set_callback(popup, infrared_popup_closed_callback);
    popup_set_context(popup, context);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);
}

bool infrared_scene_edit_delete_done_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypePopupClosed) {
            const InfraredEditTarget edit_target = infrared->app_state.edit_target;
            if(edit_target == InfraredEditTargetButton) {
                scene_manager_search_and_switch_to_previous_scene(
                    scene_manager, InfraredSceneRemote);
            } else if(edit_target == InfraredEditTargetRemote) {
                const uint32_t possible_scenes[] = {InfraredSceneStart, InfraredSceneRemoteList};
                if(!scene_manager_search_and_switch_to_previous_scene_one_of(
                       scene_manager, possible_scenes, COUNT_OF(possible_scenes))) {
                    view_dispatcher_stop(infrared->view_dispatcher);
                }
            } else {
                furi_crash();
            }
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_edit_delete_done_on_exit(void* context) {
    InfraredApp* infrared = context;
    UNUSED(infrared);
}
