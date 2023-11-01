#include "../infrared_app_i.h"

void infrared_scene_error_databases_on_enter(void* context) {
    InfraredApp* infrared = context;
    Popup* popup = infrared->popup;

    popup_set_icon(popup, 5, 11, &I_SDQuestion_35x43);
    popup_set_text(
        popup, "Function requires\nSD card with fresh\ndatabases.", 47, 17, AlignLeft, AlignTop);

    popup_set_context(popup, context);
    popup_set_callback(popup, infrared_popup_closed_callback);

    infrared_play_notification_message(infrared, InfraredNotificationMessageYellowOn);
    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);
}

bool infrared_scene_error_databases_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypePopupClosed) {
            scene_manager_search_and_switch_to_previous_scene(
                infrared->scene_manager, InfraredSceneUniversal);
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_error_databases_on_exit(void* context) {
    InfraredApp* infrared = context;
    popup_reset(infrared->popup);
    infrared_play_notification_message(infrared, InfraredNotificationMessageYellowOff);
}
