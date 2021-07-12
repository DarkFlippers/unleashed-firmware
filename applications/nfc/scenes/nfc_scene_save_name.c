#include "../nfc_i.h"

#define SCENE_SAVE_NAME_CUSTOM_EVENT (0UL)

void nfc_scene_save_name_text_input_callback(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(
        nfc->nfc_common.view_dispatcher, SCENE_SAVE_NAME_CUSTOM_EVENT);
}

const void nfc_scene_save_name_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    TextInput* text_input = nfc->text_input;
    nfc_text_store_clear(nfc);
    text_input_set_header_text(text_input, "Name the card");
    text_input_set_result_callback(
        text_input,
        nfc_scene_save_name_text_input_callback,
        nfc,
        nfc->text_store,
        sizeof(nfc->text_store));
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewTextInput);
}

const bool nfc_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_SAVE_NAME_CUSTOM_EVENT) {
            memcpy(&nfc->device.dev_name, nfc->text_store, strlen(nfc->text_store));
            if(nfc_device_save(&nfc->device, nfc->text_store)) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveSuccess);
                return true;
            } else {
                return scene_manager_search_previous_scene(nfc->scene_manager, NfcSceneStart);
            }
        }
    }
    return false;
}

const void nfc_scene_save_name_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear view
    text_input_set_header_text(nfc->text_input, NULL);
}
