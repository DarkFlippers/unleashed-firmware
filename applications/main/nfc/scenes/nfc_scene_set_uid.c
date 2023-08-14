#include "../nfc_i.h"

void nfc_scene_set_uid_byte_input_callback(void* context) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventByteInputDone);
}

void nfc_scene_set_uid_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup view
    ByteInput* byte_input = nfc->byte_input;
    byte_input_set_header_text(byte_input, "Enter UID in hex");
    nfc->dev_edit_data = nfc->dev->dev_data.nfc_data;
    byte_input_set_result_callback(
        byte_input,
        nfc_scene_set_uid_byte_input_callback,
        NULL,
        nfc,
        nfc->dev_edit_data.uid,
        nfc->dev_edit_data.uid_len);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewByteInput);
}

bool nfc_scene_set_uid_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            if(scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneSavedMenu)) {
                nfc->dev->dev_data.nfc_data = nfc->dev_edit_data;
                if(nfc_save_file(nfc)) {
                    scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveSuccess);
                    consumed = true;
                }
            } else if(scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneSetTypeMfUid)) {
                MfClassicType mf_type =
                    scene_manager_get_scene_state(nfc->scene_manager, NfcSceneSetTypeMfUid);
                if(mf_type > MfClassicTypeMini) {
                    furi_crash("Nfc unknown type");
                }
                nfc_generate_mf_classic_ext(
                    &nfc->dev->dev_data,
                    nfc->dev_edit_data.uid_len,
                    mf_type,
                    false,
                    nfc->dev_edit_data.uid);
                scene_manager_next_scene(nfc->scene_manager, NfcSceneGenerateInfo);
                consumed = true;

            } else {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveName);
                consumed = true;
            }
        }
    }

    return consumed;
}

void nfc_scene_set_uid_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    byte_input_set_result_callback(nfc->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(nfc->byte_input, "");
}
