#include "nfc_scene_debug_read_mifare_ul.h"
#include "../nfc_i.h"

#include <furi.h>

const void nfc_scene_debug_read_mifare_ul_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewMifareUl);
}

const bool nfc_scene_debug_read_mifare_ul_on_event(void* context, uint32_t event) {
    return false;
}

const void nfc_scene_debug_read_mifare_ul_on_exit(void* context) {
}

AppScene* nfc_scene_debug_read_mifare_ul_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneDebugReadMifareUl;
    scene->on_enter = nfc_scene_debug_read_mifare_ul_on_enter;
    scene->on_event = nfc_scene_debug_read_mifare_ul_on_event;
    scene->on_exit = nfc_scene_debug_read_mifare_ul_on_exit;

    return scene;
}

void nfc_scene_debug_read_mifare_ul_free(AppScene* scene) {
    free(scene);
}