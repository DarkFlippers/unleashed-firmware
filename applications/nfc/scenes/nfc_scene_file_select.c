#include "../nfc_i.h"

void nfc_scene_file_select_on_enter(void* context) {
    Nfc* nfc = context;
    // Process file_select return
    if(nfc_file_select(nfc->dev)) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneSavedMenu);
    } else {
        scene_manager_search_and_switch_to_previous_scene(nfc->scene_manager, NfcSceneStart);
    }
}

bool nfc_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_scene_file_select_on_exit(void* context) {
    UNUSED(context);
}
