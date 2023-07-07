#include "../nfc_i.h"
#include <dolphin/dolphin.h>

void nfc_scene_nfcv_key_input_byte_input_callback(void* context) {
    Nfc* nfc = context;
    NfcVSlixData* data = &nfc->dev->dev_data.nfcv_data.sub_data.slix;

    memcpy(data->key_privacy, nfc->byte_input_store, 4);
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventByteInputDone);
}

void nfc_scene_nfcv_key_input_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup view
    ByteInput* byte_input = nfc->byte_input;
    byte_input_set_header_text(byte_input, "Enter The Password In Hex");
    byte_input_set_result_callback(
        byte_input,
        nfc_scene_nfcv_key_input_byte_input_callback,
        NULL,
        nfc,
        nfc->byte_input_store,
        4);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewByteInput);
}

bool nfc_scene_nfcv_key_input_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneNfcVUnlock);
            dolphin_deed(DolphinDeedNfcRead);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_nfcv_key_input_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    byte_input_set_result_callback(nfc->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(nfc->byte_input, "");
}
