#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"

static void nfc_scene_set_uid_byte_input_changed_callback(void* context) {
    NfcApp* instance = context;
    // Retrieve previously saved UID length
    const size_t uid_len = scene_manager_get_scene_state(instance->scene_manager, NfcSceneSetUid);
    nfc_device_set_uid(instance->nfc_device, instance->byte_input_store, uid_len);
}

void nfc_scene_set_uid_on_enter(void* context) {
    NfcApp* instance = context;

    size_t uid_len;
    const uint8_t* uid = nfc_device_get_uid(instance->nfc_device, &uid_len);

    memcpy(instance->byte_input_store, uid, uid_len);
    // Save UID length for use in callback
    scene_manager_set_scene_state(instance->scene_manager, NfcSceneSetUid, uid_len);

    // Setup view
    ByteInput* byte_input = instance->byte_input;
    byte_input_set_header_text(byte_input, "Enter UID in hex");
    byte_input_set_result_callback(
        byte_input,
        nfc_protocol_support_common_byte_input_done_callback,
        nfc_scene_set_uid_byte_input_changed_callback,
        instance,
        instance->byte_input_store,
        uid_len);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewByteInput);
}

bool nfc_scene_set_uid_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            if(scene_manager_has_previous_scene(instance->scene_manager, NfcSceneSavedMenu)) {
                if(nfc_save(instance)) {
                    scene_manager_next_scene(instance->scene_manager, NfcSceneSaveSuccess);
                    consumed = true;
                }
            } else if(scene_manager_has_previous_scene(instance->scene_manager, NfcSceneReadMenu)) {
                scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, NfcSceneReadMenu);
                consumed = true;
            } else {
                scene_manager_next_scene(instance->scene_manager, NfcSceneSaveName);
                consumed = true;
            }
        }
    }

    return consumed;
}

void nfc_scene_set_uid_on_exit(void* context) {
    NfcApp* instance = context;

    // Clear view
    byte_input_set_result_callback(instance->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(instance->byte_input, "");
}
