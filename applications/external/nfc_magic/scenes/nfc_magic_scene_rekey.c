#include "../nfc_magic_i.h"

enum {
    NfcMagicSceneRekeyStateCardSearch,
    NfcMagicSceneRekeyStateCardFound,
};

bool nfc_magic_rekey_worker_callback(NfcMagicWorkerEvent event, void* context) {
    furi_assert(context);

    NfcMagic* nfc_magic = context;
    view_dispatcher_send_custom_event(nfc_magic->view_dispatcher, event);

    return true;
}

static void nfc_magic_scene_rekey_setup_view(NfcMagic* nfc_magic) {
    Popup* popup = nfc_magic->popup;
    popup_reset(popup);
    uint32_t state = scene_manager_get_scene_state(nfc_magic->scene_manager, NfcMagicSceneRekey);

    if(state == NfcMagicSceneRekeyStateCardSearch) {
        popup_set_text(
            nfc_magic->popup,
            "Apply the\nsame card\nto the back",
            128,
            32,
            AlignRight,
            AlignCenter);
        popup_set_icon(nfc_magic->popup, 0, 8, &I_NFC_manual_60x50);
    } else {
        popup_set_icon(popup, 12, 23, &I_Loading_24);
        popup_set_header(popup, "Writing\nDon't move...", 52, 32, AlignLeft, AlignCenter);
    }

    view_dispatcher_switch_to_view(nfc_magic->view_dispatcher, NfcMagicViewPopup);
}

void nfc_magic_scene_rekey_on_enter(void* context) {
    NfcMagic* nfc_magic = context;

    scene_manager_set_scene_state(
        nfc_magic->scene_manager, NfcMagicSceneRekey, NfcMagicSceneRekeyStateCardSearch);
    nfc_magic_scene_rekey_setup_view(nfc_magic);

    // Setup and start worker
    nfc_magic_worker_start(
        nfc_magic->worker,
        NfcMagicWorkerStateRekey,
        nfc_magic->dev,
        &nfc_magic->source_dev->dev_data,
        nfc_magic->new_password,
        nfc_magic_rekey_worker_callback,
        nfc_magic);
    nfc_magic_blink_start(nfc_magic);
}

bool nfc_magic_scene_rekey_on_event(void* context, SceneManagerEvent event) {
    NfcMagic* nfc_magic = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcMagicWorkerEventSuccess) {
            nfc_magic->dev->password = nfc_magic->new_password;
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneSuccess);
            consumed = true;
        } else if(event.event == NfcMagicWorkerEventFail) {
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneRekeyFail);
            consumed = true;
        } else if(event.event == NfcMagicWorkerEventCardDetected) {
            scene_manager_set_scene_state(
                nfc_magic->scene_manager, NfcMagicSceneRekey, NfcMagicSceneRekeyStateCardFound);
            nfc_magic_scene_rekey_setup_view(nfc_magic);
            consumed = true;
        } else if(event.event == NfcMagicWorkerEventNoCardDetected) {
            scene_manager_set_scene_state(
                nfc_magic->scene_manager, NfcMagicSceneRekey, NfcMagicSceneRekeyStateCardSearch);
            nfc_magic_scene_rekey_setup_view(nfc_magic);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_magic_scene_rekey_on_exit(void* context) {
    NfcMagic* nfc_magic = context;

    nfc_magic_worker_stop(nfc_magic->worker);
    scene_manager_set_scene_state(
        nfc_magic->scene_manager, NfcMagicSceneRekey, NfcMagicSceneRekeyStateCardSearch);
    // Clear view
    popup_reset(nfc_magic->popup);

    nfc_magic_blink_stop(nfc_magic);
}
