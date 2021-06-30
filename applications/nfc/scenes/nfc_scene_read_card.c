#include <nfc/scenes/nfc_scene_read_card.h>

#include <furi.h>

#include "../nfc_i.h"
#include "../views/nfc_detect.h"

#include <gui/view_dispatcher.h>

void nfc_read_card_worker_callback(void* context) {
    Nfc* nfc = (Nfc*)context;
    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, NfcEventDetect);
}

const void nfc_scene_read_card_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    Popup* popup = nfc->popup;
    popup_set_header(popup, "Detecting\nNFC card", 70, 34, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 4, I_RFIDDolphinReceive_98x60);

    // Start worker
    nfc_worker_start(
        nfc->nfc_common.worker,
        NfcWorkerStateDetect,
        &nfc->nfc_common.worker_result,
        nfc_read_card_worker_callback,
        nfc);
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewPopup);
}

const bool nfc_scene_read_card_on_event(void* context, uint32_t event) {
    Nfc* nfc = (Nfc*)context;

    if(event == NfcEventDetect) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_read_card_success);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    }
    return false;
}

const void nfc_scene_read_card_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Stop worker
    nfc_worker_stop(nfc->nfc_common.worker);

    // Clear view
    Popup* popup = nfc->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, I_Empty_1x1);
}

AppScene* nfc_scene_read_card_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneReadCard;
    scene->on_enter = nfc_scene_read_card_on_enter;
    scene->on_event = nfc_scene_read_card_on_event;
    scene->on_exit = nfc_scene_read_card_on_exit;

    return scene;
}

void nfc_scene_read_card_free(AppScene* scene) {
    free(scene);
}
