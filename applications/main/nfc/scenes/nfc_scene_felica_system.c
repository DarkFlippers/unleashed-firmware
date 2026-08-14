#include "../helpers/protocol_support/felica/felica_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_felica_system_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(FelicaExtraSceneSystem, context);
}

bool nfc_scene_felica_system_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(FelicaExtraSceneSystem, context, event);
}

void nfc_scene_felica_system_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(FelicaExtraSceneSystem, context);
}
