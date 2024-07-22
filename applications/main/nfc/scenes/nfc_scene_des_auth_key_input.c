#include "../nfc_app_i.h"

void nfc_scene_des_auth_key_input_byte_input_callback(void* context) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventByteInputDone);
}

void nfc_scene_des_auth_key_input_on_enter(void* context) {
    NfcApp* nfc = context;

    // Setup view
    NfcProtocol protocol = nfc_device_get_protocol(nfc->nfc_device);
    uint8_t* key = (protocol == NfcProtocolFelica) ? nfc->felica_auth->card_key.data :
                                                     nfc->mf_ul_auth->tdes_key.data;

    ByteInput* byte_input = nfc->byte_input;
    byte_input_set_header_text(byte_input, "Enter key in hex");
    byte_input_set_result_callback(
        byte_input,
        nfc_scene_des_auth_key_input_byte_input_callback,
        NULL,
        nfc,
        key,
        FELICA_DATA_BLOCK_SIZE);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewByteInput);
}

bool nfc_scene_des_auth_key_input_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    UNUSED(event);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            NfcProtocol protocol = nfc_device_get_protocol(nfc->nfc_device);

            if(protocol == NfcProtocolFelica) {
                nfc->felica_auth->skip_auth = false;
            } else {
                nfc->mf_ul_auth->type = MfUltralightAuthTypeManual;
            }

            scene_manager_next_scene(nfc->scene_manager, NfcSceneDesAuthUnlockWarn);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_des_auth_key_input_on_exit(void* context) {
    NfcApp* nfc = context;

    // Clear view
    byte_input_set_result_callback(nfc->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(nfc->byte_input, "");
}
