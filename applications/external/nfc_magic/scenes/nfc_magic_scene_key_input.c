#include "../nfc_magic_i.h"

void nfc_magic_scene_key_input_byte_input_callback(void* context) {
    NfcMagic* nfc_magic = context;

    view_dispatcher_send_custom_event(
        nfc_magic->view_dispatcher, NfcMagicCustomEventByteInputDone);
}

void nfc_magic_scene_key_input_on_enter(void* context) {
    NfcMagic* nfc_magic = context;

    // Setup view
    ByteInput* byte_input = nfc_magic->byte_input;
    byte_input_set_header_text(byte_input, "Enter the password in hex");
    byte_input_set_result_callback(
        byte_input,
        nfc_magic_scene_key_input_byte_input_callback,
        NULL,
        nfc_magic,
        (uint8_t*)&nfc_magic->dev->password,
        4);
    view_dispatcher_switch_to_view(nfc_magic->view_dispatcher, NfcMagicViewByteInput);
}

bool nfc_magic_scene_key_input_on_event(void* context, SceneManagerEvent event) {
    NfcMagic* nfc_magic = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcMagicCustomEventByteInputDone) {
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneCheck);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_magic_scene_key_input_on_exit(void* context) {
    NfcMagic* nfc_magic = context;

    // Clear view
    byte_input_set_result_callback(nfc_magic->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(nfc_magic->byte_input, "");
}
