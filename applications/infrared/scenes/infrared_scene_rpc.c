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
        } else if(event.event == InfraredCustomEventTypeRpcLoad) {
            bool result = false;
            const char* arg = rpc_system_app_get_data(infrared->rpc_ctx);
            if(arg) {
                string_set_str(infrared->file_path, arg);
                result = infrared_remote_load(infrared->remote, infrared->file_path);
                infrared_worker_tx_set_get_signal_callback(
                    infrared->worker, infrared_worker_tx_get_signal_steady_callback, infrared);
                infrared_worker_tx_set_signal_sent_callback(
                    infrared->worker, infrared_signal_sent_callback, infrared);
            }
            const char* remote_name = infrared_remote_get_name(infrared->remote);

            infrared_text_store_set(infrared, 0, "loaded\n%s", remote_name);
            popup_set_text(
                infrared->popup, infrared->text_store[0], 82, 32, AlignCenter, AlignTop);

            rpc_system_app_confirm(infrared->rpc_ctx, RpcAppEventLoadFile, result);
        } else if(event.event == InfraredCustomEventTypeRpcButtonPress) {
            bool result = false;
            const char* arg = rpc_system_app_get_data(infrared->rpc_ctx);
            if(arg) {
                size_t button_index = 0;
                if(infrared_remote_find_button_by_name(infrared->remote, arg, &button_index)) {
                    infrared_tx_start_button_index(infrared, button_index);
                    result = true;
                }
            }
            rpc_system_app_confirm(infrared->rpc_ctx, RpcAppEventButtonRelease, result);
        } else if(event.event == InfraredCustomEventTypeRpcButtonRelease) {
            infrared_tx_stop(infrared);
            rpc_system_app_confirm(infrared->rpc_ctx, RpcAppEventButtonRelease, true);
        } else if(event.event == InfraredCustomEventTypeRpcExit) {
            view_dispatcher_stop(infrared->view_dispatcher);
            rpc_system_app_confirm(infrared->rpc_ctx, RpcAppEventAppExit, true);
        } else if(event.event == InfraredCustomEventTypeRpcSessionClose) {
            rpc_system_app_set_callback(infrared->rpc_ctx, NULL, NULL);
            infrared->rpc_ctx = NULL;
            view_dispatcher_stop(infrared->view_dispatcher);
        }
    }
    return consumed;
}

void infrared_scene_rpc_on_exit(void* context) {
    Infrared* infrared = context;
    popup_reset(infrared->popup);
}
