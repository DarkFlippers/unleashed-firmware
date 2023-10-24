#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_save_name_on_enter(void* context) {
    nfc_protocol_support_on_enter(NfcProtocolSupportSceneSaveName, context);
}

bool nfc_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_on_event(NfcProtocolSupportSceneSaveName, context, event);
}

void nfc_scene_save_name_on_exit(void* context) {
    nfc_protocol_support_on_exit(NfcProtocolSupportSceneSaveName, context);
}
