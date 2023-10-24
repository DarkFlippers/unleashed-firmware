#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_saved_menu_on_enter(void* context) {
    nfc_protocol_support_on_enter(NfcProtocolSupportSceneSavedMenu, context);
}

bool nfc_scene_saved_menu_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_on_event(NfcProtocolSupportSceneSavedMenu, context, event);
}

void nfc_scene_saved_menu_on_exit(void* context) {
    nfc_protocol_support_on_exit(NfcProtocolSupportSceneSavedMenu, context);
}
