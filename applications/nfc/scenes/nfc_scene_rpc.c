#include "../nfc_i.h"

void nfc_scene_rpc_on_enter(void* context) {
    Nfc* nfc = context;
    Popup* popup = nfc->popup;

    popup_set_header(popup, "NFC", 82, 28, AlignCenter, AlignBottom);
    popup_set_text(popup, "RPC mode", 82, 32, AlignCenter, AlignTop);

    popup_set_icon(popup, 2, 14, &I_Warning_30x23); // TODO: icon

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);

    notification_message(nfc->notifications, &sequence_display_backlight_on);
}

static bool nfc_scene_rpc_emulate_callback(NfcWorkerEvent event, void* context) {
    UNUSED(event);
    Nfc* nfc = context;

    nfc->rpc_state = NfcRpcStateEmulated;
    return true;
}

bool nfc_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    Popup* popup = nfc->popup;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == NfcCustomEventViewExit) {
            rpc_system_app_confirm(nfc->rpc_ctx, RpcAppEventAppExit, true);
            view_dispatcher_stop(nfc->view_dispatcher);
            nfc_blink_stop(nfc);
        } else if(event.event == NfcCustomEventRpcSessionClose) {
            rpc_system_app_set_callback(nfc->rpc_ctx, NULL, NULL);
            nfc->rpc_ctx = NULL;
            view_dispatcher_stop(nfc->view_dispatcher);
            nfc_blink_stop(nfc);
        } else if(event.event == NfcCustomEventRpcLoad) {
            bool result = false;
            const char* arg = rpc_system_app_get_data(nfc->rpc_ctx);
            if((arg) && (nfc->rpc_state == NfcRpcStateIdle)) {
                if(nfc_device_load(nfc->dev, arg, false)) {
                    if(nfc->dev->format == NfcDeviceSaveFormatMifareUl) {
                        nfc_worker_start(
                            nfc->worker,
                            NfcWorkerStateMfUltralightEmulate,
                            &nfc->dev->dev_data,
                            nfc_scene_rpc_emulate_callback,
                            nfc);
                    } else if(nfc->dev->format == NfcDeviceSaveFormatMifareClassic) {
                        nfc_worker_start(
                            nfc->worker,
                            NfcWorkerStateMfClassicEmulate,
                            &nfc->dev->dev_data,
                            nfc_scene_rpc_emulate_callback,
                            nfc);
                    } else {
                        nfc_worker_start(
                            nfc->worker, NfcWorkerStateUidEmulate, &nfc->dev->dev_data, NULL, nfc);
                    }
                    nfc->rpc_state = NfcRpcStateEmulating;
                    result = true;

                    nfc_blink_start(nfc);
                    nfc_text_store_set(nfc, "emulating\n%s", nfc->dev->dev_name);
                    popup_set_text(popup, nfc->text_store, 82, 32, AlignCenter, AlignTop);
                }
            }

            rpc_system_app_confirm(nfc->rpc_ctx, RpcAppEventLoadFile, result);
        }
    }
    return consumed;
}

void nfc_scene_rpc_on_exit(void* context) {
    Nfc* nfc = context;
    Popup* popup = nfc->popup;

    nfc_blink_stop(nfc);

    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
