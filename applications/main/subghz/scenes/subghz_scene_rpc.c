#include "../subghz_i.h"
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/protocols/star_line.h>
#include <lib/subghz/protocols/alutech_at_4n.h>
#include <lib/subghz/protocols/nice_flor_s.h>
#include <lib/subghz/protocols/somfy_telis.h>

typedef enum {
    SubGhzRpcStateIdle,
    SubGhzRpcStateLoaded,
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
            rpc_system_app_confirm(subghz->rpc_ctx, RpcAppEventAppExit, true);
        } else if(event.event == SubGhzCustomEventSceneRpcSessionClose) {
            scene_manager_stop(subghz->scene_manager);
            view_dispatcher_stop(subghz->view_dispatcher);
        } else if(event.event == SubGhzCustomEventSceneRpcButtonPress) {
            bool result = false;
            if((subghz->txrx->txrx_state == SubGhzTxRxStateSleep) &&
               (state == SubGhzRpcStateLoaded)) {
                result = subghz_tx_start(subghz, subghz->txrx->fff_data);
                if(result) subghz_blink_start(subghz);
            }
            if(!result) {
                rpc_system_app_set_error_code(subghz->rpc_ctx, SubGhzErrorTypeOnlyRX);
                rpc_system_app_set_error_text(
                    subghz->rpc_ctx,
                    "Transmission on this frequency is restricted in your settings");
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
            if(arg && (state == SubGhzRpcStateIdle)) {
                if(subghz_key_load(subghz, arg, false)) {
                    scene_manager_set_scene_state(
                        subghz->scene_manager, SubGhzSceneRpc, SubGhzRpcStateLoaded);
                    furi_string_set(subghz->file_path, arg);
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
            rpc_system_app_confirm(subghz->rpc_ctx, RpcAppEventLoadFile, result);
        }
    }
    return consumed;
}

void subghz_scene_rpc_on_exit(void* context) {
    SubGhz* subghz = context;

    if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
        subghz_tx_stop(subghz);
        subghz_sleep(subghz);
        subghz_blink_stop(subghz);
    }

    Popup* popup = subghz->popup;

    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);

    keeloq_reset_mfname();
    keeloq_reset_kl_type();
    keeloq_reset_original_btn();
    alutech_reset_original_btn();
    nice_flors_reset_original_btn();
    somfy_telis_reset_original_btn();
    star_line_reset_mfname();
    star_line_reset_kl_type();
}
