#include "../subghz_i.h"

void subghz_scene_rpc_on_enter(void* context) {
    SubGhz* subghz = context;
    Popup* popup = subghz->popup;

    popup_set_header(popup, "Sub-GHz", 82, 28, AlignCenter, AlignBottom);
    popup_set_text(popup, "RPC mode", 82, 32, AlignCenter, AlignTop);

    popup_set_icon(popup, 2, 14, &I_Warning_30x23); // TODO: icon

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdPopup);

    notification_message(subghz->notifications, &sequence_display_backlight_on);
}

bool subghz_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    Popup* popup = subghz->popup;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == SubGhzCustomEventSceneExit) {
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
            }
            view_dispatcher_stop(subghz->view_dispatcher);
            rpc_system_app_confirm(subghz->rpc_ctx, RpcAppEventAppExit, true);
        } else if(event.event == SubGhzCustomEventSceneRpcSessionClose) {
            rpc_system_app_set_callback(subghz->rpc_ctx, NULL, NULL);
            subghz->rpc_ctx = NULL;
            subghz_blink_stop(subghz);
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
            }
            view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneExit);
        } else if(event.event == SubGhzCustomEventSceneRpcButtonPress) {
            bool result = false;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateSleep) {
                subghz_blink_start(subghz);
                result = subghz_tx_start(subghz, subghz->txrx->fff_data);
                result = true;
            }
            rpc_system_app_confirm(subghz->rpc_ctx, RpcAppEventButtonPress, result);
        } else if(event.event == SubGhzCustomEventSceneRpcButtonRelease) {
            bool result = false;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_blink_stop(subghz);
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
                result = true;
            }
            rpc_system_app_confirm(subghz->rpc_ctx, RpcAppEventButtonRelease, result);
        } else if(event.event == SubGhzCustomEventSceneRpcLoad) {
            bool result = false;
            const char* arg = rpc_system_app_get_data(subghz->rpc_ctx);
            if(arg) {
                if(subghz_key_load(subghz, arg, false)) {
                    string_set_str(subghz->file_path, arg);
                    result = true;
                    string_t file_name;
                    string_init(file_name);
                    path_extract_filename(subghz->file_path, file_name, true);

                    snprintf(
                        subghz->file_name_tmp,
                        SUBGHZ_MAX_LEN_NAME,
                        "loaded\n%s",
                        string_get_cstr(file_name));
                    popup_set_text(popup, subghz->file_name_tmp, 82, 32, AlignCenter, AlignTop);

                    string_clear(file_name);
                }
            }
            rpc_system_app_confirm(subghz->rpc_ctx, RpcAppEventLoadFile, result);
        }
    }
    return consumed;
}

void subghz_scene_rpc_on_exit(void* context) {
    SubGhz* subghz = context;
    Popup* popup = subghz->popup;

    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
