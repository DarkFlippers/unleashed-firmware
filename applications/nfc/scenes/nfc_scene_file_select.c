#include <nfc/scenes/nfc_scene_file_select.h>

#include <furi.h>

#include "../nfc_i.h"

const void nfc_scene_file_select_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    // Process file_select return
    if(nfc_file_select(&nfc->device)) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_saved_menu);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
    } else {
        view_dispatcher_send_back_search_scene_event(
            nfc->nfc_common.view_dispatcher, NfcSceneStart);
    }
}

const bool nfc_scene_file_select_on_event(void* context, uint32_t event) {
    return false;
}

const void nfc_scene_file_select_on_exit(void* context) {
}

AppScene* nfc_scene_file_select_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneFileSelect;
    scene->on_enter = nfc_scene_file_select_on_enter;
    scene->on_event = nfc_scene_file_select_on_event;
    scene->on_exit = nfc_scene_file_select_on_exit;

    return scene;
}

void nfc_scene_file_select_free(AppScene* scene) {
    free(scene);
}
