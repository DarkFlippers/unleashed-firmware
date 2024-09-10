#include "../ibutton_i.h"

void ibutton_scene_rpc_on_enter(void* context) {
    UNUSED(context);
}

static void ibutton_rpc_start_emulation(iButton* ibutton) {
    Popup* popup = ibutton->popup;

    popup_set_header(popup, "iButton", 82, 28, AlignCenter, AlignBottom);
    popup_set_text(popup, ibutton->key_name, 82, 32, AlignCenter, AlignTop);
    popup_set_icon(popup, 2, 14, &I_iButtonKey_49x44);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);

    ibutton_worker_emulate_start(ibutton->worker, ibutton->key);

    ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateStart);
    notification_message(ibutton->notifications, &sequence_display_backlight_on);
}

bool ibutton_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;

        if(event.event == iButtonCustomEventRpcLoadFile) {
            bool result = false;

            if(ibutton_load_key(ibutton, false)) {
                ibutton_rpc_start_emulation(ibutton);
                result = true;
            } else {
                rpc_system_app_set_error_code(ibutton->rpc, RpcAppSystemErrorCodeParseFile);
                rpc_system_app_set_error_text(ibutton->rpc, "Cannot load key file");
            }
            rpc_system_app_confirm(ibutton->rpc, result);
        } else if(event.event == iButtonCustomEventRpcExit) {
            rpc_system_app_confirm(ibutton->rpc, true);
            scene_manager_stop(ibutton->scene_manager);
            view_dispatcher_stop(ibutton->view_dispatcher);

        } else if(event.event == iButtonCustomEventRpcSessionClose) {
            scene_manager_stop(ibutton->scene_manager);
            view_dispatcher_stop(ibutton->view_dispatcher);
        }
    }

    return consumed;
}

void ibutton_scene_rpc_on_exit(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;

    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);

    ibutton_notification_message(ibutton, iButtonNotificationMessageBlinkStop);
}
