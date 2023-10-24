#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"

static void nfc_scene_set_sak_byte_input_changed_callback(void* context) {
    NfcApp* instance = context;
    iso14443_3a_set_sak(instance->iso14443_3a_edit_data, instance->byte_input_store[0]);
}

void nfc_scene_set_sak_on_enter(void* context) {
    NfcApp* instance = context;

    instance->byte_input_store[0] = iso14443_3a_get_sak(instance->iso14443_3a_edit_data);

    // Setup view
    ByteInput* byte_input = instance->byte_input;
    byte_input_set_header_text(byte_input, "Enter SAK in hex");
    byte_input_set_result_callback(
        byte_input,
        nfc_protocol_support_common_byte_input_done_callback,
        nfc_scene_set_sak_byte_input_changed_callback,
        instance,
        instance->byte_input_store,
        1);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewByteInput);
}

bool nfc_scene_set_sak_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneSetAtqa);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_set_sak_on_exit(void* context) {
    NfcApp* instance = context;

    // Clear view
    byte_input_set_result_callback(instance->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(instance->byte_input, "");
}
