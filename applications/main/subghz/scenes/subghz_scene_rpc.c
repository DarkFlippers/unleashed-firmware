#include "../subghz_i.h"

typedef enum {
    SubGhzRpcStateIdle,
    SubGhzRpcStateLoaded,
    SubGhzRpcStateTx,
} SubGhzRpcState;

void subghz_scene_rpc_on_enter(void* context) {
    SubGhz* subghz = context;
    Popup* popup = subghz->popup;

    popup_set_header(popup, "Sub-GHz", 89, 42, AlignCenter, AlignBottom);
    popup_set_text(popup, "RPC mode", 89, 44, AlignCenter, AlignTop);

    popup_set_icon(popup, 0, 12, &I_RFIDDolphinSend_97x61);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdPopup);

    scene_manager_set_scene_state(subghz->scene_manager, SubGhzSceneRpc, SubGhzRpcStateIdle);

    notification_message(subghz->notifications, &sequence_display_backlight_on);
}

bool subghz_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    Popup* popup = subghz->popup;
    bool consumed = false;
    SubGhzRpcState state = scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneRpc);

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == SubGhzCustomEventSceneExit) {
            scene_manager_stop(subghz->scene_manager);
            view_dispatcher_stop(subghz->view_dispatcher);
            rpc_system_app_confirm(subghz->rpc_ctx, true);
        } else if(event.event == SubGhzCustomEventSceneRpcSessionClose) {
            scene_manager_stop(subghz->scene_manager);
            view_dispatcher_stop(subghz->view_dispatcher);
        } else if(event.event == SubGhzCustomEventSceneRpcButtonPress) {
            bool result = false;
            if(state == SubGhzRpcStateLoaded) {
                switch(
                    subghz_txrx_tx_start(subghz->txrx, subghz_txrx_get_fff_data(subghz->txrx))) {
                case SubGhzTxRxStartTxStateErrorOnlyRx:
                    rpc_system_app_set_error_code(subghz->rpc_ctx, SubGhzErrorTypeOnlyRX);
                    rpc_system_app_set_error_text(
                        subghz->rpc_ctx,
                        "Transmission on this frequency is restricted in your region");
                    break;
                case SubGhzTxRxStartTxStateErrorParserOthers:
                    rpc_system_app_set_error_code(subghz->rpc_ctx, SubGhzErrorTypeParserOthers);
                    rpc_system_app_set_error_text(
                        subghz->rpc_ctx, "Error in protocol parameters description");
                    break;

                default: //if(SubGhzTxRxStartTxStateOk)
                    result = true;
                    subghz_blink_start(subghz);
                    state = SubGhzRpcStateTx;
                    break;
                }
            }
            rpc_system_app_confirm(subghz->rpc_ctx, result);
        } else if(event.event == SubGhzCustomEventSceneRpcButtonRelease) {
            bool result = false;
            if(state == SubGhzRpcStateTx) {
                subghz_txrx_stop(subghz->txrx);
                subghz_blink_stop(subghz);
                result = true;
            }
            state = SubGhzRpcStateIdle;
            rpc_system_app_confirm(subghz->rpc_ctx, result);
        } else if(event.event == SubGhzCustomEventSceneRpcLoad) {
            bool result = false;
            if(state == SubGhzRpcStateIdle) {
                if(subghz_key_load(subghz, furi_string_get_cstr(subghz->file_path), false)) {
                    scene_manager_set_scene_state(
                        subghz->scene_manager, SubGhzSceneRpc, SubGhzRpcStateLoaded);
                    result = true;
                    FuriString* file_name;
                    file_name = furi_string_alloc();
                    path_extract_filename(subghz->file_path, file_name, true);

                    snprintf(
                        subghz->file_name_tmp,
                        SUBGHZ_MAX_LEN_NAME,
                        "loaded\n%s",
                        furi_string_get_cstr(file_name));
                    popup_set_text(popup, subghz->file_name_tmp, 89, 44, AlignCenter, AlignTop);

                    furi_string_free(file_name);
                } else {
                    rpc_system_app_set_error_code(subghz->rpc_ctx, SubGhzErrorTypeParseFile);
                    rpc_system_app_set_error_text(subghz->rpc_ctx, "Cannot parse file");
                }
            }
            rpc_system_app_confirm(subghz->rpc_ctx, result);
        }
    }
    return consumed;
}

void subghz_scene_rpc_on_exit(void* context) {
    SubGhz* subghz = context;
    SubGhzRpcState state = scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneRpc);
    if(state == SubGhzRpcStateTx) {
        subghz_txrx_stop(subghz->txrx);
        subghz_blink_stop(subghz);
    }

    Popup* popup = subghz->popup;

    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
