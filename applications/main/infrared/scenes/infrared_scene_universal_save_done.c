#include "../infrared_app_i.h"

#include "common/infrared_scene_universal_common.h"

void infrared_scene_universal_save_done_on_enter(void* context) {
    InfraredApp* infrared = context;
    Popup* popup = infrared->popup;

    popup_set_icon(popup, 36, 5, &I_DolphinSaved_92x58);
    popup_set_header(popup, "Saved", 15, 19, AlignLeft, AlignBottom);
    popup_set_callback(popup, infrared_popup_closed_callback);
    popup_set_context(popup, context);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    infrared_play_notification_message(infrared, InfraredNotificationMessageSuccess);
    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);
}

bool infrared_scene_universal_save_done_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypePopupClosed) {
            infrared_scene_universal_common_return(infrared);
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_universal_save_done_on_exit(void* context) {
    InfraredApp* infrared = context;
    popup_reset(infrared->popup);
}
