#include "../nfc_i.h"

#define SCENE_SET_UID_CUSTOM_EVENT (0UL)

void nfc_scene_set_uid_byte_input_callback(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, SCENE_SET_UID_CUSTOM_EVENT);
}

void nfc_scene_set_uid_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    ByteInput* byte_input = nfc->byte_input;
    byte_input_set_header_text(byte_input, "Enter uid in hex");
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

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_SET_UID_CUSTOM_EVENT) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveName);
            return true;
        }
    }
    return false;
}

void nfc_scene_set_uid_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear view
    byte_input_set_result_callback(nfc->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(nfc->byte_input, "");
}
