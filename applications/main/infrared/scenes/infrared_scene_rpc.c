#include "../infrared_i.h"
#include <gui/canvas.h>

typedef enum {
    InfraredRpcStateIdle,
    InfraredRpcStateLoaded,
    InfraredRpcStateSending,
} InfraredRpcState;

void infrared_scene_rpc_on_enter(void* context) {
    Infrared* infrared = context;
    Popup* popup = infrared->popup;

    popup_set_header(popup, "Infrared", 89, 42, AlignCenter, AlignBottom);
    popup_set_text(popup, "RPC mode", 89, 44, AlignCenter, AlignTop);

    popup_set_icon(popup, 0, 12, &I_RFIDDolphinSend_97x61);

    popup_set_context(popup, context);
    popup_set_callback(popup, infrared_popup_closed_callback);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);

    scene_manager_set_scene_state(infrared->scene_manager, InfraredSceneRpc, InfraredRpcStateIdle);

    notification_message(infrared->notifications, &sequence_display_backlight_on);
}

bool infrared_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    Infrared* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        InfraredRpcState state =
            scene_manager_get_scene_state(infrared->scene_manager, InfraredSceneRpc);
        if(event.event == InfraredCustomEventTypeBackPressed) {
            view_dispatcher_stop(infrared->view_dispatcher);
        } else if(event.event == InfraredCustomEventTypePopupClosed) {
            view_dispatcher_stop(infrared->view_dispatcher);
        } else if(event.event == InfraredCustomEventTypeRpcLoad) {
            bool result = false;
            const char* arg = rpc_system_app_get_data(infrared->rpc_ctx);
            if(arg && (state == InfraredRpcStateIdle)) {
                furi_string_set(infrared->file_path, arg);
                result = infrared_remote_load(infrared->remote, infrared->file_path);
                if(result) {
                    scene_manager_set_scene_state(
                        infrared->scene_manager, InfraredSceneRpc, InfraredRpcStateLoaded);
                }
            }
            const char* remote_name = infrared_remote_get_name(infrared->remote);

            infrared_text_store_set(infrared, 0, "loaded\n%s", remote_name);
            popup_set_text(
                infrared->popup, infrared->text_store[0], 89, 44, AlignCenter, AlignTop);

            rpc_system_app_confirm(infrared->rpc_ctx, RpcAppEventLoadFile, result);
        } else if(event.event == InfraredCustomEventTypeRpcButtonPress) {
            bool result = false;
            const char* arg = rpc_system_app_get_data(infrared->rpc_ctx);
            if(arg && (state == InfraredRpcStateLoaded)) {
                size_t button_index = 0;
                if(infrared_remote_find_button_by_name(infrared->remote, arg, &button_index)) {
                    infrared_tx_start_button_index(infrared, button_index);
                    result = true;
                    scene_manager_set_scene_state(
                        infrared->scene_manager, InfraredSceneRpc, InfraredRpcStateSending);
                }
            }
            rpc_system_app_confirm(infrared->rpc_ctx, RpcAppEventButtonRelease, result);
        } else if(event.event == InfraredCustomEventTypeRpcButtonRelease) {
            bool result = false;
            if(state == InfraredRpcStateSending) {
                infrared_tx_stop(infrared);
                result = true;
                scene_manager_set_scene_state(
                    infrared->scene_manager, InfraredSceneRpc, InfraredRpcStateLoaded);
            }
            rpc_system_app_confirm(infrared->rpc_ctx, RpcAppEventButtonRelease, result);
        } else if(event.event == InfraredCustomEventTypeRpcExit) {
            scene_manager_stop(infrared->scene_manager);
            view_dispatcher_stop(infrared->view_dispatcher);
            rpc_system_app_confirm(infrared->rpc_ctx, RpcAppEventAppExit, true);
        } else if(event.event == InfraredCustomEventTypeRpcSessionClose) {
            scene_manager_stop(infrared->scene_manager);
            view_dispatcher_stop(infrared->view_dispatcher);
        }
    }
    return consumed;
}

void infrared_scene_rpc_on_exit(void* context) {
    Infrared* infrared = context;
    if(scene_manager_get_scene_state(infrared->scene_manager, InfraredSceneRpc) ==
       InfraredRpcStateSending) {
        infrared_tx_stop(infrared);
    }
    popup_reset(infrared->popup);
}
