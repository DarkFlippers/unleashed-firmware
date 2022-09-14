#include "../nfc_i.h"
#include "m-string.h"
#include <lib/toolbox/random_name.h>
#include <gui/modules/validators.h>
#include <toolbox/path.h>

void nfc_scene_save_name_text_input_callback(void* context) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventTextInputDone);
}

void nfc_scene_save_name_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup view
    TextInput* text_input = nfc->text_input;
    bool dev_name_empty = false;
    if(!strcmp(nfc->dev->dev_name, "")) {
        set_random_name(nfc->text_store, sizeof(nfc->text_store));
        dev_name_empty = true;
    } else {
        nfc_text_store_set(nfc, nfc->dev->dev_name);
    }
    text_input_set_header_text(text_input, "Name the card");
    text_input_set_result_callback(
        text_input,
        nfc_scene_save_name_text_input_callback,
        nfc,
        nfc->text_store,
        NFC_DEV_NAME_MAX_LEN,
        dev_name_empty);

    string_t folder_path;
    string_init(folder_path);

    if(string_end_with_str_p(nfc->dev->load_path, NFC_APP_EXTENSION)) {
        path_extract_dirname(string_get_cstr(nfc->dev->load_path), folder_path);
    } else {
        string_set_str(folder_path, NFC_APP_FOLDER);
    }

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        string_get_cstr(folder_path), NFC_APP_EXTENSION, nfc->dev->dev_name);
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextInput);

    string_clear(folder_path);
}

bool nfc_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventTextInputDone) {
            if(strcmp(nfc->dev->dev_name, "")) {
                nfc_device_delete(nfc->dev, true);
            }
            if(scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneSetUid)) {
                nfc->dev->dev_data.nfc_data = nfc->dev_edit_data;
            }
            strlcpy(nfc->dev->dev_name, nfc->text_store, strlen(nfc->text_store) + 1);
            if(nfc_device_save(nfc->dev, nfc->text_store)) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveSuccess);
                consumed = true;
            } else {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneStart);
            }
        }
    }
    return consumed;
}

void nfc_scene_save_name_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    void* validator_context = text_input_get_validator_callback_context(nfc->text_input);
    text_input_set_validator(nfc->text_input, NULL, NULL);
    validator_is_file_free(validator_context);

    text_input_reset(nfc->text_input);
}
