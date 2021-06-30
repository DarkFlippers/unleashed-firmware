#include "nfc_scene_debug_detect.h"
#include "../nfc_i.h"

#include <furi.h>

const void nfc_scene_debug_detect_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewDetect);
}

const bool nfc_scene_debug_detect_on_event(void* context, uint32_t event) {
    return false;
}

const void nfc_scene_debug_detect_on_exit(void* context) {
}

AppScene* nfc_scene_debug_detect_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneDebugDetect;
    scene->on_enter = nfc_scene_debug_detect_on_enter;
    scene->on_event = nfc_scene_debug_detect_on_event;
    scene->on_exit = nfc_scene_debug_detect_on_exit;

    return scene;
}

void nfc_scene_debug_detect_free(AppScene* scene) {
    free(scene);
}