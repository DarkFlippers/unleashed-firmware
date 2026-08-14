#include "../helpers/protocol_support/mf_classic/mf_classic_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_mf_classic_mfkey_complete_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(MfClassicExtraSceneMfkeyComplete, context);
}

bool nfc_scene_mf_classic_mfkey_complete_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(MfClassicExtraSceneMfkeyComplete, context, event);
}

void nfc_scene_mf_classic_mfkey_complete_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(MfClassicExtraSceneMfkeyComplete, context);
}
