#include "../nfc_i.h"
#include <lib/toolbox/random_name.h>
#include <gui/modules/validators.h>
#include <toolbox/path.h>

void nfc_scene_passport_auth_save_name_text_input_callback(void* context) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventTextInputDone);
}

void nfc_scene_passport_auth_save_name_on_enter(void* context) {
    Nfc* nfc = context;

    MrtdData* mrtd_data = &nfc->dev->dev_data.mrtd_data;

    // Setup view
    TextInput* text_input = nfc->text_input;
    bool docnr_empty = false;
    if(!strcmp(mrtd_data->auth.doc_number, "")) {
        set_random_name(nfc->text_store, sizeof(nfc->text_store));
        docnr_empty = true;
    } else {
        nfc_text_store_set(nfc, mrtd_data->auth.doc_number);
    }
    text_input_set_header_text(text_input, "Name the parameters");
    text_input_set_result_callback(
        text_input,
        nfc_scene_passport_auth_save_name_text_input_callback,
        nfc,
        nfc->text_store,
        NFC_DEV_NAME_MAX_LEN,
        docnr_empty);

    FuriString* folder_path;
    folder_path = furi_string_alloc();

    if(furi_string_end_with(nfc->dev->load_path, NFC_APP_EXTENSION)) {
        path_extract_dirname(furi_string_get_cstr(nfc->dev->load_path), folder_path);
    } else {
        furi_string_set(folder_path, NFC_APP_FOLDER);
    }

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(furi_string_get_cstr(folder_path), NFC_APP_EXTENSION, NULL);
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextInput);

    furi_string_free(folder_path);
}

bool nfc_scene_passport_auth_save_name_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    MrtdData* mrtd_data = &nfc->dev->dev_data.mrtd_data;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventTextInputDone) {
            if(mrtd_auth_params_save(
                   nfc->dev->storage, nfc->dev->dialogs, &mrtd_data->auth, nfc->text_store)) {
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

void nfc_scene_passport_auth_save_name_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    void* validator_context = text_input_get_validator_callback_context(nfc->text_input);
    text_input_set_validator(nfc->text_input, NULL, NULL);
    validator_is_file_free(validator_context);

    text_input_reset(nfc->text_input);
}
