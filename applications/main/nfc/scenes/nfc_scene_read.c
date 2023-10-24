#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_read_on_enter(void* context) {
    nfc_protocol_support_on_enter(NfcProtocolSupportSceneRead, context);
}

bool nfc_scene_read_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_on_event(NfcProtocolSupportSceneRead, context, event);
}

void nfc_scene_read_on_exit(void* context) {
    nfc_protocol_support_on_exit(NfcProtocolSupportSceneRead, context);
}
