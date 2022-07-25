#include "../infrared_i.h"
#include "gui/canvas.h"

void infrared_scene_rpc_on_enter(void* context) {
    Infrared* infrared = context;
    Popup* popup = infrared->popup;

    popup_set_header(popup, "Infrared", 82, 28, AlignCenter, AlignBottom);
    popup_set_text(popup, "RPC mode", 82, 32, AlignCenter, AlignTop);

    popup_set_icon(popup, 2, 14, &I_Warning_30x23); // TODO: icon

    popup_set_context(popup, context);
    popup_set_callback(popup, infrared_popup_closed_callback);

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
        } else if(event.event == InfraredCustomEventTypeRpcLoaded) {
            const char* remote_name = infrared_remote_get_name(infrared->remote);

            infrared_text_store_set(infrared, 0, "loaded\n%s", remote_name);
            popup_set_text(
                infrared->popup, infrared->text_store[0], 82, 32, AlignCenter, AlignTop);
        }
    }
    return consumed;
}

void infrared_scene_rpc_on_exit(void* context) {
    Infrared* infrared = context;
    popup_reset(infrared->popup);
}
