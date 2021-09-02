#include "../nfc_i.h"
#include <lib/toolbox/random_name.h>

#define SCENE_SAVE_NAME_CUSTOM_EVENT (0UL)

void nfc_scene_save_name_text_input_callback(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, SCENE_SAVE_NAME_CUSTOM_EVENT);
}

const void nfc_scene_save_name_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    TextInput* text_input = nfc->text_input;
    bool dev_name_empty = false;
    if(!strcmp(nfc->dev.dev_name, "")) {
        set_random_name(nfc->text_store, sizeof(nfc->text_store));
        dev_name_empty = true;
    } else {
        nfc_text_store_set(nfc, nfc->dev.dev_name);
    }
    text_input_set_header_text(text_input, "Name the card");
    text_input_set_result_callback(
        text_input,
        nfc_scene_save_name_text_input_callback,
        nfc,
        nfc->text_store,
        NFC_DEV_NAME_MAX_LEN,
        dev_name_empty);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextInput);
}

const bool nfc_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_SAVE_NAME_CUSTOM_EVENT) {
            if(nfc->dev.dev_name) {
                nfc_device_delete(&nfc->dev);
            }
            if(scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneSetUid)) {
                nfc->dev.dev_data.nfc_data = nfc->dev_edit_data;
            }
            memcpy(&nfc->dev.dev_name, nfc->text_store, strlen(nfc->text_store));
            if(nfc_device_save(&nfc->dev, nfc->text_store)) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveSuccess);
                return true;
            } else {
                return scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneStart);
            }
        }
    }
    return false;
}

const void nfc_scene_save_name_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear view
    text_input_set_header_text(nfc->text_input, NULL);
    text_input_set_result_callback(nfc->text_input, NULL, NULL, NULL, 0, false);
}
