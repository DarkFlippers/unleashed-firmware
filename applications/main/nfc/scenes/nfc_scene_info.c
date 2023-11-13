#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_info_on_enter(void* context) {
    nfc_protocol_support_on_enter(NfcProtocolSupportSceneInfo, context);
}

bool nfc_scene_info_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_on_event(NfcProtocolSupportSceneInfo, context, event);
}

void nfc_scene_info_on_exit(void* context) {
    nfc_protocol_support_on_exit(NfcProtocolSupportSceneInfo, context);
}
