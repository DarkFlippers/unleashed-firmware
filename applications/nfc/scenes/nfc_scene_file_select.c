#include "../nfc_i.h"

const void nfc_scene_file_select_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    // Process file_select return
    if(nfc_file_select(&nfc->device)) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneSavedMenu);
    } else {
        scene_manager_search_previous_scene(nfc->scene_manager, NfcSceneStart);
    }
}

const bool nfc_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    return false;
}

const void nfc_scene_file_select_on_exit(void* context) {
}
