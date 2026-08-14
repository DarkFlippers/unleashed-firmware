#include "../helpers/protocol_support/slix/slix_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_slix_unlock_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(NfcProtocolSlix, SlixExtraSceneUnlock, context);
}

bool nfc_scene_slix_unlock_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(
        NfcProtocolSlix, SlixExtraSceneUnlock, context, event);
}

void nfc_scene_slix_unlock_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(NfcProtocolSlix, SlixExtraSceneUnlock, context);
}
