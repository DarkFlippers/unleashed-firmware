#include "../nfc_i.h"

const void nfc_scene_debug_read_emv_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewEmv);
}

const bool nfc_scene_debug_read_emv_on_event(void* context, SceneManagerEvent event) {
    return false;
}

const void nfc_scene_debug_read_emv_on_exit(void* context) {
}
