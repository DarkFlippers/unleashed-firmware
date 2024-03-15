#include "../infrared_app_i.h"
#include <gui/canvas.h>

#define TAG "InfraredApp"

typedef enum {
    InfraredRpcStateIdle,
    InfraredRpcStateLoaded,
    InfraredRpcStateSending,
} InfraredRpcState;

static int32_t infrared_scene_rpc_task_callback(void* context) {
    InfraredApp* infrared = context;
    const bool success =
        infrared_remote_load(infrared->remote, furi_string_get_cstr(infrared->file_path));
    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypeTaskFinished);
    return success;
}

void infrared_scene_rpc_on_enter(void* context) {
    InfraredApp* infrared = context;
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
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        InfraredAppState* app_state = &infrared->app_state;
        InfraredRpcState rpc_state =
            scene_manager_get_scene_state(infrared->scene_manager, InfraredSceneRpc);

        if(event.event == InfraredCustomEventTypeRpcLoadFile) {
            if(rpc_state == InfraredRpcStateIdle) {
                // Load the remote in a separate thread
                infrared_blocking_task_start(infrared, infrared_scene_rpc_task_callback);
            }

        } else if(event.event == InfraredCustomEventTypeTaskFinished) {
            const bool task_success = infrared_blocking_task_finalize(infrared);

            if(task_success) {
                const char* remote_name = infrared_remote_get_name(infrared->remote);
                infrared_text_store_set(infrared, 0, "loaded\n%s", remote_name);
                scene_manager_set_scene_state(
                    infrared->scene_manager, InfraredSceneRpc, InfraredRpcStateLoaded);

            } else {
                infrared_text_store_set(
                    infrared, 0, "failed to load\n%s", furi_string_get_cstr(infrared->file_path));
            }

            popup_set_text(
                infrared->popup, infrared->text_store[0], 89, 44, AlignCenter, AlignTop);
            view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);

            rpc_system_app_confirm(infrared->rpc_ctx, task_success);

        } else if(
            event.event == InfraredCustomEventTypeRpcButtonPressName ||
            event.event == InfraredCustomEventTypeRpcButtonPressIndex) {
            bool result = false;
            if(rpc_state == InfraredRpcStateLoaded) {
                if(event.event == InfraredCustomEventTypeRpcButtonPressName) {
                    const char* button_name = furi_string_get_cstr(infrared->button_name);
                    size_t index;
                    const bool index_found =
                        infrared_remote_get_signal_index(infrared->remote, button_name, &index);
                    app_state->current_button_index = index_found ? (signed)index :
                                                                    InfraredButtonIndexNone;
                    FURI_LOG_D(TAG, "Sending signal with name \"%s\"", button_name);
                } else {
                    FURI_LOG_D(
                        TAG, "Sending signal with index \"%ld\"", app_state->current_button_index);
                }
                if(infrared->app_state.current_button_index != InfraredButtonIndexNone) {
                    infrared_tx_start_button_index(infrared, app_state->current_button_index);
                    scene_manager_set_scene_state(
                        infrared->scene_manager, InfraredSceneRpc, InfraredRpcStateSending);
                    result = true;
                }
            }
            rpc_system_app_confirm(infrared->rpc_ctx, result);

        } else if(event.event == InfraredCustomEventTypeRpcButtonRelease) {
            bool result = false;

            if(rpc_state == InfraredRpcStateSending) {
                infrared_tx_stop(infrared);
                result = true;
                scene_manager_set_scene_state(
                    infrared->scene_manager, InfraredSceneRpc, InfraredRpcStateLoaded);
            }

            rpc_system_app_confirm(infrared->rpc_ctx, result);

        } else if(
            event.event == InfraredCustomEventTypeRpcExit ||
            event.event == InfraredCustomEventTypeRpcSessionClose ||
            event.event == InfraredCustomEventTypePopupClosed) {
            scene_manager_stop(infrared->scene_manager);
            view_dispatcher_stop(infrared->view_dispatcher);

            if(event.event == InfraredCustomEventTypeRpcExit) {
                rpc_system_app_confirm(infrared->rpc_ctx, true);
            }
        }

        consumed = true;
    }

    return consumed;
}

void infrared_scene_rpc_on_exit(void* context) {
    InfraredApp* infrared = context;
    if(scene_manager_get_scene_state(infrared->scene_manager, InfraredSceneRpc) ==
       InfraredRpcStateSending) {
        infrared_tx_stop(infrared);
    }

    popup_reset(infrared->popup);
}
