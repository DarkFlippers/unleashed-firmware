#include "../infrared_i.h"
#include "gui/canvas.h"

void infrared_scene_rpc_on_enter(void* context) {
    Infrared* infrared = context;
    Popup* popup = infrared->popup;

    popup_set_text(popup, "Rpc mode", 64, 28, AlignCenter, AlignCenter);

    popup_set_context(popup, context);
    popup_set_callback(popup, infrared_popup_closed_callback);

    infrared_play_notification_message(infrared, InfraredNotificationMessageYellowOn);
    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);

    notification_message(infrared->notifications, &sequence_display_backlight_on);
}

bool infrared_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    Infrared* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == InfraredCustomEventTypeBackPressed) {
            view_dispatcher_stop(infrared->view_dispatcher);
        } else if(event.event == InfraredCustomEventTypePopupClosed) {
            view_dispatcher_stop(infrared->view_dispatcher);
        }
    }
    return consumed;
}

void infrared_scene_rpc_on_exit(void* context) {
    Infrared* infrared = context;
    popup_reset(infrared->popup);
}
