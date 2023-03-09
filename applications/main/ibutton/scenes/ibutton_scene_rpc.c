#include "../ibutton_i.h"

void ibutton_scene_rpc_on_enter(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;

    popup_set_header(popup, "iButton", 82, 28, AlignCenter, AlignBottom);
    popup_set_text(popup, "RPC mode", 82, 32, AlignCenter, AlignTop);

    popup_set_icon(popup, 2, 14, &I_iButtonKey_49x44);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);

    notification_message(ibutton->notifications, &sequence_display_backlight_on);
}

bool ibutton_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;

        if(event.event == iButtonCustomEventRpcLoad) {
            bool result = false;
            const char* file_path = rpc_system_app_get_data(ibutton->rpc);

            if(file_path && (furi_string_empty(ibutton->file_path))) {
                furi_string_set(ibutton->file_path, file_path);

                if(ibutton_load_key(ibutton)) {
                    popup_set_text(popup, ibutton->key_name, 82, 32, AlignCenter, AlignTop);
                    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);

                    ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateStart);
                    ibutton_worker_emulate_start(ibutton->worker, ibutton->key);

                    result = true;
                }
            }

            rpc_system_app_confirm(ibutton->rpc, RpcAppEventLoadFile, result);

        } else if(event.event == iButtonCustomEventRpcExit) {
            rpc_system_app_confirm(ibutton->rpc, RpcAppEventAppExit, true);
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
