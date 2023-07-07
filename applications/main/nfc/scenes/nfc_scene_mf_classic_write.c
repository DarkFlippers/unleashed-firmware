#include "../nfc_i.h"
#include <dolphin/dolphin.h>

enum {
    NfcSceneMfClassicWriteStateCardSearch,
    NfcSceneMfClassicWriteStateCardFound,
};

bool nfc_mf_classic_write_worker_callback(NfcWorkerEvent event, void* context) {
    furi_assert(context);

    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, event);

    return true;
}

static void nfc_scene_mf_classic_write_setup_view(Nfc* nfc) {
    Popup* popup = nfc->popup;
    popup_reset(popup);
    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfClassicWrite);

    if(state == NfcSceneMfClassicWriteStateCardSearch) {
        popup_set_text(
            nfc->popup, "Apply the initial\ncard only", 128, 32, AlignRight, AlignCenter);
        popup_set_icon(nfc->popup, 0, 8, &I_NFC_manual_60x50);
    } else {
        popup_set_header(popup, "Writing\nDon't move...", 52, 32, AlignLeft, AlignCenter);
        popup_set_icon(popup, 12, 23, &A_Loading_24);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
}

void nfc_scene_mf_classic_write_on_enter(void* context) {
    Nfc* nfc = context;
    dolphin_deed(DolphinDeedNfcEmulate);

    scene_manager_set_scene_state(
        nfc->scene_manager, NfcSceneMfClassicWrite, NfcSceneMfClassicWriteStateCardSearch);
    nfc_scene_mf_classic_write_setup_view(nfc);

    // Setup and start worker
    nfc_worker_start(
        nfc->worker,
        NfcWorkerStateMfClassicWrite,
        &nfc->dev->dev_data,
        nfc_mf_classic_write_worker_callback,
        nfc);
    nfc_blink_emulate_start(nfc);
}

bool nfc_scene_mf_classic_write_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcWorkerEventSuccess) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicWriteSuccess);
            consumed = true;
        } else if(event.event == NfcWorkerEventFail) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicWriteFail);
            consumed = true;
        } else if(event.event == NfcWorkerEventWrongCard) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicWrongCard);
            consumed = true;
        } else if(event.event == NfcWorkerEventCardDetected) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfClassicWrite, NfcSceneMfClassicWriteStateCardFound);
            nfc_scene_mf_classic_write_setup_view(nfc);
            consumed = true;
        } else if(event.event == NfcWorkerEventNoCardDetected) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfClassicWrite, NfcSceneMfClassicWriteStateCardSearch);
            nfc_scene_mf_classic_write_setup_view(nfc);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_mf_classic_write_on_exit(void* context) {
    Nfc* nfc = context;

    nfc_worker_stop(nfc->worker);
    scene_manager_set_scene_state(
        nfc->scene_manager, NfcSceneMfClassicWrite, NfcSceneMfClassicWriteStateCardSearch);
    // Clear view
    popup_reset(nfc->popup);

    nfc_blink_stop(nfc);
}
