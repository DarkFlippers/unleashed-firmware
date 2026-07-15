#include "../nfc_app_i.h"

#include <nfc/protocols/mf_plus/mf_plus.h>

void nfc_scene_mf_plus_keys_add_byte_input_callback(void* context) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventByteInputDone);
}

void nfc_scene_mf_plus_keys_add_on_enter(void* context) {
    NfcApp* instance = context;

    ByteInput* byte_input = instance->byte_input;
    byte_input_set_header_text(byte_input, "Enter the key in hex");
    byte_input_set_result_callback(
        byte_input,
        nfc_scene_mf_plus_keys_add_byte_input_callback,
        NULL,
        instance,
        instance->byte_input_store,
        sizeof(MfPlusKey));
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewByteInput);
}

bool nfc_scene_mf_plus_keys_add_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            KeysDict* dict = keys_dict_alloc(
                NFC_APP_MF_PLUS_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfPlusKey));

            MfPlusKey key = {};
            memcpy(key.data, instance->byte_input_store, sizeof(MfPlusKey));
            if(keys_dict_is_key_present(dict, key.data, sizeof(MfPlusKey))) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusKeysWarnDuplicate);
            } else if(keys_dict_add_key(dict, key.data, sizeof(MfPlusKey))) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneSaveSuccess);
                dolphin_deed(DolphinDeedNfcKeyAdd);
            } else {
                scene_manager_previous_scene(instance->scene_manager);
            }

            keys_dict_free(dict);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_mf_plus_keys_add_on_exit(void* context) {
    NfcApp* instance = context;

    byte_input_set_result_callback(instance->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(instance->byte_input, "");
}
