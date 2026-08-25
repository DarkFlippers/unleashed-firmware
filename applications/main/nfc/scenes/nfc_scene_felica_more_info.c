#include "../helpers/protocol_support/felica/felica_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_felica_more_info_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(NfcProtocolFelica, FelicaExtraSceneMoreInfo, context);
}

bool nfc_scene_felica_more_info_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(
        NfcProtocolFelica, FelicaExtraSceneMoreInfo, context, event);
}

void nfc_scene_felica_more_info_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(NfcProtocolFelica, FelicaExtraSceneMoreInfo, context);
}
