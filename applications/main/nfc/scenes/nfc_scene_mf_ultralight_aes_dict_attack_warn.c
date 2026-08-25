#include "../helpers/protocol_support/mf_ultralight/mf_ultralight_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_mf_ultralight_aes_dict_attack_warn_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(
        NfcProtocolMfUltralight, MfUltralightExtraSceneAesDictAttackWarn, context);
}

bool nfc_scene_mf_ultralight_aes_dict_attack_warn_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(
        NfcProtocolMfUltralight, MfUltralightExtraSceneAesDictAttackWarn, context, event);
}

void nfc_scene_mf_ultralight_aes_dict_attack_warn_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(
        NfcProtocolMfUltralight, MfUltralightExtraSceneAesDictAttackWarn, context);
}
