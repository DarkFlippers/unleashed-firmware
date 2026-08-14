#include "../helpers/protocol_support/mf_ultralight/mf_ultralight_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_mf_ultralight_unlock_menu_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(MfUltralightExtraSceneUnlockMenu, context);
}

bool nfc_scene_mf_ultralight_unlock_menu_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(MfUltralightExtraSceneUnlockMenu, context, event);
}

void nfc_scene_mf_ultralight_unlock_menu_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(MfUltralightExtraSceneUnlockMenu, context);
}
