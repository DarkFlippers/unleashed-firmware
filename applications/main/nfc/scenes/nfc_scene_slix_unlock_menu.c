#include "../helpers/protocol_support/slix/slix_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_slix_unlock_menu_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(SlixExtraSceneUnlockMenu, context);
}

bool nfc_scene_slix_unlock_menu_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(SlixExtraSceneUnlockMenu, context, event);
}

void nfc_scene_slix_unlock_menu_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(SlixExtraSceneUnlockMenu, context);
}
