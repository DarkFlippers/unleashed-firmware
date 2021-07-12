#include "../nfc_i.h"

const void nfc_scene_debug_read_mifare_ul_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewMifareUl);
}

const bool nfc_scene_debug_read_mifare_ul_on_event(void* context, SceneManagerEvent event) {
    return false;
}

const void nfc_scene_debug_read_mifare_ul_on_exit(void* context) {
}
