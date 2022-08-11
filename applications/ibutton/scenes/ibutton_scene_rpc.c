#include "../ibutton_i.h"
#include <toolbox/path.h>
#include <rpc/rpc_app.h>

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
    UNUSED(context);
    UNUSED(event);
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventRpcLoad) {
            const char* arg = rpc_system_app_get_data(ibutton->rpc_ctx);
            bool result = false;
            if(arg && (string_empty_p(ibutton->file_path))) {
                string_set_str(ibutton->file_path, arg);
                if(ibutton_load_key_data(ibutton, ibutton->file_path, false)) {
                    ibutton_worker_emulate_start(ibutton->key_worker, ibutton->key);
                    string_t key_name;
                    string_init(key_name);
                    if(string_end_with_str_p(ibutton->file_path, IBUTTON_APP_EXTENSION)) {
                        path_extract_filename(ibutton->file_path, key_name, true);
                    }

                    if(!string_empty_p(key_name)) {
                        ibutton_text_store_set(
                            ibutton, "emulating\n%s", string_get_cstr(key_name));
                    } else {
                        ibutton_text_store_set(ibutton, "emulating");
                    }
                    popup_set_text(popup, ibutton->text_store, 82, 32, AlignCenter, AlignTop);

                    ibutton_notification_message(ibutton, iButtonNotificationMessageEmulateStart);

                    string_clear(key_name);
                    result = true;
                } else {
                    string_reset(ibutton->file_path);
                }
            }
            rpc_system_app_confirm(ibutton->rpc_ctx, RpcAppEventLoadFile, result);
        } else if(event.event == iButtonCustomEventRpcExit) {
            rpc_system_app_confirm(ibutton->rpc_ctx, RpcAppEventAppExit, true);
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
