#include "../nfc_app_i.h"

#define TAG "NfcKeyDict"

// Load the key at user-dictionary index `key_index`. Returns false if the index no longer
// exists (e.g. the file changed under us).
static bool
    nfc_scene_key_dict_delete_load_key(const NfcKeyDict* dict, uint32_t key_index, uint8_t* key) {
    KeysDict* user_dict = keys_dict_alloc(dict->user_path, KeysDictModeOpenAlways, dict->key_size);
    bool loaded = key_index < keys_dict_get_total_keys(user_dict);
    for(size_t i = 0; loaded && i < (key_index + 1); i++) {
        loaded = keys_dict_get_next_key(user_dict, key, dict->key_size);
    }
    keys_dict_free(user_dict);
    return loaded;
}

void nfc_scene_key_dict_delete_widget_callback(GuiButtonType result, InputType type, void* context) {
    NfcApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_scene_key_dict_delete_on_enter(void* context) {
    NfcApp* instance = context;
    const NfcKeyDict* dict = nfc_key_dict(instance->key_dict_type);

    uint32_t key_index =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneKeyDictDelete);

    uint8_t key[NFC_BYTE_INPUT_STORE_SIZE];
    const bool key_loaded = nfc_scene_key_dict_delete_load_key(dict, key_index, key);

    widget_add_string_element(
        instance->widget,
        64,
        0,
        AlignCenter,
        AlignTop,
        FontPrimary,
        key_loaded ? "Delete this key?" : "Key Not Found");
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeLeft,
        key_loaded ? "Cancel" : "Back",
        nfc_scene_key_dict_delete_widget_callback,
        instance);

    if(key_loaded) {
        // Only offer Delete for a key we could read back. Otherwise the dialog would ask the user
        // to confirm deleting a blank line, and act on a stale index if they did.
        widget_add_button_element(
            instance->widget,
            GuiButtonTypeRight,
            "Delete",
            nfc_scene_key_dict_delete_widget_callback,
            instance);

        FuriString* key_str = furi_string_alloc();
        for(size_t i = 0; i < dict->key_size; i++) {
            furi_string_cat_printf(key_str, "%02X", key[i]);
        }
        widget_add_string_element(
            instance->widget,
            64,
            32,
            AlignCenter,
            AlignCenter,
            FontSecondary,
            furi_string_get_cstr(key_str));
        furi_string_free(key_str);
    } else {
        FURI_LOG_W(TAG, "Key %lu is gone from %s", key_index, dict->user_path);
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_key_dict_delete_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            const NfcKeyDict* dict = nfc_key_dict(instance->key_dict_type);
            uint32_t key_index =
                scene_manager_get_scene_state(instance->scene_manager, NfcSceneKeyDictDelete);
            uint8_t key[NFC_BYTE_INPUT_STORE_SIZE];
            bool deleted = false;
            if(nfc_scene_key_dict_delete_load_key(dict, key_index, key)) {
                KeysDict* user_dict =
                    keys_dict_alloc(dict->user_path, KeysDictModeOpenAlways, dict->key_size);
                deleted = keys_dict_delete_key(user_dict, key, dict->key_size);
                keys_dict_free(user_dict);
            }
            if(deleted) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneDeleteSuccess);
            } else {
                FURI_LOG_E(TAG, "Failed to delete key %lu from %s", key_index, dict->user_path);
                notification_message(instance->notifications, &sequence_error);
                scene_manager_previous_scene(instance->scene_manager);
            }
        } else if(event.event == GuiButtonTypeLeft) {
            scene_manager_previous_scene(instance->scene_manager);
        }
        consumed = true;
    }

    return consumed;
}

void nfc_scene_key_dict_delete_on_exit(void* context) {
    NfcApp* instance = context;

    widget_reset(instance->widget);
}
