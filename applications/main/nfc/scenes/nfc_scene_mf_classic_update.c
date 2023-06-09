#include "../nfc_i.h"
#include <dolphin/dolphin.h>

enum {
    NfcSceneMfClassicUpdateStateCardSearch,
    NfcSceneMfClassicUpdateStateCardFound,
};

bool nfc_mf_classic_update_worker_callback(NfcWorkerEvent event, void* context) {
    furi_assert(context);

    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, event);

    return true;
}

static void nfc_scene_mf_classic_update_setup_view(Nfc* nfc) {
    Popup* popup = nfc->popup;
    popup_reset(popup);
    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfClassicUpdate);

    if(state == NfcSceneMfClassicUpdateStateCardSearch) {
        popup_set_text(
            nfc->popup, "Apply the initial\ncard only", 128, 32, AlignRight, AlignCenter);
        popup_set_icon(nfc->popup, 0, 8, &I_NFC_manual_60x50);
    } else {
        popup_set_header(popup, "Updating\nDon't move...", 52, 32, AlignLeft, AlignCenter);
        popup_set_icon(popup, 12, 23, &A_Loading_24);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
}

void nfc_scene_mf_classic_update_on_enter(void* context) {
    Nfc* nfc = context;
    dolphin_deed(DolphinDeedNfcEmulate);

    scene_manager_set_scene_state(
        nfc->scene_manager, NfcSceneMfClassicUpdate, NfcSceneMfClassicUpdateStateCardSearch);
    nfc_scene_mf_classic_update_setup_view(nfc);

    // Setup and start worker
    nfc_worker_start(
        nfc->worker,
        NfcWorkerStateMfClassicUpdate,
        &nfc->dev->dev_data,
        nfc_mf_classic_update_worker_callback,
        nfc);
    nfc_blink_emulate_start(nfc);
}

bool nfc_scene_mf_classic_update_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcWorkerEventSuccess) {
            nfc_worker_stop(nfc->worker);
            if(nfc_device_save_shadow(nfc->dev, furi_string_get_cstr(nfc->dev->load_path))) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicUpdateSuccess);
            } else {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicWrongCard);
            }
            consumed = true;
        } else if(event.event == NfcWorkerEventWrongCard) {
            nfc_worker_stop(nfc->worker);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicWrongCard);
            consumed = true;
        } else if(event.event == NfcWorkerEventCardDetected) {
            scene_manager_set_scene_state(
                nfc->scene_manager,
                NfcSceneMfClassicUpdate,
                NfcSceneMfClassicUpdateStateCardFound);
            nfc_scene_mf_classic_update_setup_view(nfc);
            consumed = true;
        } else if(event.event == NfcWorkerEventNoCardDetected) {
            scene_manager_set_scene_state(
                nfc->scene_manager,
                NfcSceneMfClassicUpdate,
                NfcSceneMfClassicUpdateStateCardSearch);
            nfc_scene_mf_classic_update_setup_view(nfc);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_mf_classic_update_on_exit(void* context) {
    Nfc* nfc = context;
    nfc_worker_stop(nfc->worker);
    scene_manager_set_scene_state(
        nfc->scene_manager, NfcSceneMfClassicUpdate, NfcSceneMfClassicUpdateStateCardSearch);
    // Clear view
    popup_reset(nfc->popup);

    nfc_blink_stop(nfc);
}
