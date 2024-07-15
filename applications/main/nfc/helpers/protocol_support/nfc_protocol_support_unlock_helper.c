#include "nfc_protocol_support_unlock_helper.h"

static void nfc_scene_read_setup_view(NfcApp* instance) {
    Popup* popup = instance->popup;
    popup_reset(popup);
    uint32_t state = scene_manager_get_scene_state(instance->scene_manager, NfcSceneRead);

    if(state == NfcSceneReadMenuStateCardSearch) {
        popup_set_icon(instance->popup, 0, 8, &I_NFC_manual_60x50);
        popup_set_header(instance->popup, "Unlocking", 97, 15, AlignCenter, AlignTop);
        popup_set_text(
            instance->popup, "Hold card next\nto Flipper's back", 94, 27, AlignCenter, AlignTop);
    } else {
        popup_set_header(instance->popup, "Don't move", 85, 27, AlignCenter, AlignTop);
        popup_set_icon(instance->popup, 12, 20, &A_Loading_24);
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);
}

void nfc_unlock_helper_setup_from_state(NfcApp* instance) {
    bool unlocking =
        scene_manager_has_previous_scene(
            instance->scene_manager, NfcSceneMfUltralightUnlockWarn) ||
        scene_manager_has_previous_scene(instance->scene_manager, NfcSceneDesAuthUnlockWarn);

    uint32_t state = unlocking ? NfcSceneReadMenuStateCardSearch : NfcSceneReadMenuStateCardFound;

    scene_manager_set_scene_state(instance->scene_manager, NfcSceneRead, state);

    nfc_scene_read_setup_view(instance);
}

void nfc_unlock_helper_card_detected_handler(NfcApp* instance) {
    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneRead, NfcSceneReadMenuStateCardFound);
    nfc_scene_read_setup_view(instance);
}
