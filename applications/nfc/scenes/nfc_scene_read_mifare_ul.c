#include <nfc/scenes/nfc_scene_read_mifare_ul.h>
#include <furi.h>
#include "../nfc_i.h"

void nfc_read_mifare_ul_worker_callback(void* context) {
    Nfc* nfc = (Nfc*)context;
    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, NfcEventMifareUl);
}

const void nfc_scene_read_mifare_ul_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    Popup* popup = nfc->popup;
    popup_set_header(popup, "Detecting\nultralight", 70, 34, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinReceive_97x61);

    // Start worker
    nfc_worker_start(
        nfc->nfc_common.worker,
        NfcWorkerStateReadMfUltralight,
        &nfc->nfc_common.worker_result,
        nfc_read_mifare_ul_worker_callback,
        nfc);
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewPopup);
}

const bool nfc_scene_read_mifare_ul_on_event(void* context, uint32_t event) {
    Nfc* nfc = (Nfc*)context;

    if(event == NfcEventMifareUl) {
        nfc->device.data = nfc->nfc_common.worker_result.nfc_detect_data;
        view_dispatcher_add_scene(
            nfc->nfc_common.view_dispatcher, nfc->scene_read_mifare_ul_success);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    }
    return false;
}

const void nfc_scene_read_mifare_ul_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Stop worker
    nfc_worker_stop(nfc->nfc_common.worker);

    // Clear view
    Popup* popup = nfc->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}

AppScene* nfc_scene_read_mifare_ul_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneReadMifareUl;
    scene->on_enter = nfc_scene_read_mifare_ul_on_enter;
    scene->on_event = nfc_scene_read_mifare_ul_on_event;
    scene->on_exit = nfc_scene_read_mifare_ul_on_exit;

    return scene;
}

void nfc_scene_read_mifare_ul_free(AppScene* scene) {
    free(scene);
}
