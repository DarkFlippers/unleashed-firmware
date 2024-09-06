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
    const InfraredErrorCode error =
        infrared_remote_load(infrared->remote, furi_string_get_cstr(infrared->file_path));
    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypeTaskFinished);
    return error;
}

void infrared_scene_rpc_on_enter(void* context) {
    InfraredApp* infrared = context;
    scene_manager_set_scene_state(infrared->scene_manager, InfraredSceneRpc, InfraredRpcStateIdle);
}

static void infrared_scene_rpc_show(InfraredApp* infrared) {
    Popup* popup = infrared->popup;

    popup_set_header(popup, "Infrared", 89, 42, AlignCenter, AlignBottom);
    popup_set_text(popup, "RPC mode", 89, 44, AlignCenter, AlignTop);
    popup_set_text(popup, infrared->text_store[0], 89, 44, AlignCenter, AlignTop);

    popup_set_icon(popup, 0, 12, &I_RFIDDolphinSend_97x61);

    popup_set_context(popup, infrared);
    popup_set_callback(popup, infrared_popup_closed_callback);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);
    scene_manager_set_scene_state(
        infrared->scene_manager, InfraredSceneRpc, InfraredRpcStateSending);
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
            const InfraredErrorCode task_error = infrared_blocking_task_finalize(infrared);

            if(!INFRARED_ERROR_PRESENT(task_error)) {
                const char* remote_name = infrared_remote_get_name(infrared->remote);
                infrared_text_store_set(infrared, 0, "loaded\n%s", remote_name);
                scene_manager_set_scene_state(
                    infrared->scene_manager, InfraredSceneRpc, InfraredRpcStateLoaded);
            } else {
                FuriString* str = furi_string_alloc();
                furi_string_printf(
                    str, "Failed to load\n%s", furi_string_get_cstr(infrared->file_path));

                rpc_system_app_set_error_code(infrared->rpc_ctx, RpcAppSystemErrorCodeParseFile);
                rpc_system_app_set_error_text(infrared->rpc_ctx, furi_string_get_cstr(str));

                furi_string_free(str);
            }
            rpc_system_app_confirm(infrared->rpc_ctx, !INFRARED_ERROR_PRESENT(task_error));
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
                    InfraredErrorCode error =
                        infrared_tx_start_button_index(infrared, app_state->current_button_index);
                    if(!INFRARED_ERROR_PRESENT(error)) {
                        const char* remote_name = infrared_remote_get_name(infrared->remote);
                        infrared_text_store_set(infrared, 0, "emulating\n%s", remote_name);

                        infrared_scene_rpc_show(infrared);
                        result = true;
                    } else {
                        rpc_system_app_set_error_code(
                            infrared->rpc_ctx, RpcAppSystemErrorCodeInternalParse);
                        rpc_system_app_set_error_text(
                            infrared->rpc_ctx, "Cannot load button data");
                        result = false;
                    }
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
