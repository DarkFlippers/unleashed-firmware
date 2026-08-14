#include "../helpers/protocol_support/mf_desfire/mf_desfire_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_mf_desfire_app_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(MfDesfireExtraSceneApp, context);
}

bool nfc_scene_mf_desfire_app_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(MfDesfireExtraSceneApp, context, event);
}

void nfc_scene_mf_desfire_app_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(MfDesfireExtraSceneApp, context);
}
