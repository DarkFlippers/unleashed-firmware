#include "../helpers/protocol_support/mf_classic/mf_classic_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_mf_classic_detect_reader_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(MfClassicExtraSceneDetectReader, context);
}

bool nfc_scene_mf_classic_detect_reader_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(MfClassicExtraSceneDetectReader, context, event);
}

void nfc_scene_mf_classic_detect_reader_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(MfClassicExtraSceneDetectReader, context);
}
