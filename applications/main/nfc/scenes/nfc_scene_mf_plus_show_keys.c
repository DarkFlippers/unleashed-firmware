#include "../helpers/protocol_support/mf_plus/mf_plus_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_mf_plus_show_keys_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(NfcProtocolMfPlus, MfPlusExtraSceneShowKeys, context);
}

bool nfc_scene_mf_plus_show_keys_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(
        NfcProtocolMfPlus, MfPlusExtraSceneShowKeys, context, event);
}

void nfc_scene_mf_plus_show_keys_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(NfcProtocolMfPlus, MfPlusExtraSceneShowKeys, context);
}
