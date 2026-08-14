#include "../nfc_app_i.h"

void nfc_scene_key_dict_add_byte_input_callback(void* context) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventByteInputDone);
}

void nfc_scene_key_dict_add_on_enter(void* context) {
    NfcApp* instance = context;
    const NfcKeyDict* dict = nfc_key_dict(instance->key_dict_type);

    // Setup view
    ByteInput* byte_input = instance->byte_input;
    byte_input_set_header_text(byte_input, "Enter the key in hex");
    byte_input_set_result_callback(
        byte_input,
        nfc_scene_key_dict_add_byte_input_callback,
        NULL,
        instance,
        instance->byte_input_store,
        dict->key_size);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewByteInput);
}

bool nfc_scene_key_dict_add_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            const NfcKeyDict* dict = nfc_key_dict(instance->key_dict_type);

            KeysDict* user_dict =
                keys_dict_alloc(dict->user_path, KeysDictModeOpenAlways, dict->key_size);

            if(keys_dict_is_key_present(user_dict, instance->byte_input_store, dict->key_size)) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneKeyDictWarnDuplicate);
            } else if(keys_dict_add_key(user_dict, instance->byte_input_store, dict->key_size)) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneSaveSuccess);
                dolphin_deed(DolphinDeedNfcKeyAdd);
            } else {
                scene_manager_previous_scene(instance->scene_manager);
            }

            keys_dict_free(user_dict);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_key_dict_add_on_exit(void* context) {
    NfcApp* instance = context;

    // Clear view
    byte_input_set_result_callback(instance->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(instance->byte_input, "");
}
