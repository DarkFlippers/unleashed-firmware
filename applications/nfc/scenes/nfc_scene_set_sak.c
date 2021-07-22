#include "../nfc_i.h"

#define SCENE_SET_SAK_CUSTOM_EVENT (0UL)

void nfc_scene_set_sak_byte_input_callback(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, SCENE_SET_SAK_CUSTOM_EVENT);
}

const void nfc_scene_set_sak_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    ByteInput* byte_input = nfc->byte_input;
    byte_input_set_header_text(byte_input, "Enter SAK in hex");
    byte_input_set_result_callback(
        byte_input,
        nfc_scene_set_sak_byte_input_callback,
        NULL,
        nfc,
        &nfc->dev.dev_data.nfc_data.sak,
        1);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewByteInput);
}

const bool nfc_scene_set_sak_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_SET_SAK_CUSTOM_EVENT) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetAtqua);
            return true;
        }
    }
    return false;
}

const void nfc_scene_set_sak_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear view
    byte_input_set_result_callback(nfc->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(nfc->byte_input, "");
}
