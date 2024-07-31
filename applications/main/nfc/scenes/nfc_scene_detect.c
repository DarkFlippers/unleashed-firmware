#include "../nfc_app_i.h"
#include <dolphin/dolphin.h>

void nfc_scene_detect_scan_callback(NfcScannerEvent event, void* context) {
    furi_assert(context);

    NfcApp* instance = context;

    if(event.type == NfcScannerEventTypeDetected) {
        nfc_detected_protocols_set(
            instance->detected_protocols, event.data.protocols, event.data.protocol_num);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWorkerExit);
    }
}

void nfc_scene_detect_on_enter(void* context) {
    NfcApp* instance = context;

    // Setup view
    popup_reset(instance->popup);
    popup_set_header(instance->popup, "Reading", 97, 15, AlignCenter, AlignTop);
    popup_set_text(
        instance->popup, "Hold card next\nto Flipper's back", 94, 27, AlignCenter, AlignTop);
    popup_set_icon(instance->popup, 0, 8, &I_NFC_manual_60x50);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);

    nfc_detected_protocols_reset(instance->detected_protocols);

    instance->scanner = nfc_scanner_alloc(instance->nfc);
    nfc_scanner_start(instance->scanner, nfc_scene_detect_scan_callback, instance);

    nfc_blink_detect_start(instance);
}

bool nfc_scene_detect_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventWorkerExit) {
            if(nfc_detected_protocols_get_num(instance->detected_protocols) > 1) {
                notification_message(instance->notifications, &sequence_single_vibro);
                scene_manager_next_scene(instance->scene_manager, NfcSceneSelectProtocol);
            } else {
                scene_manager_next_scene(instance->scene_manager, NfcSceneRead);
            }
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_detect_on_exit(void* context) {
    NfcApp* instance = context;

    nfc_scanner_stop(instance->scanner);
    nfc_scanner_free(instance->scanner);
    popup_reset(instance->popup);

    nfc_blink_stop(instance);
}
