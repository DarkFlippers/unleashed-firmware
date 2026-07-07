#include "../nfc_app_i.h"

#include <nfc/protocols/mf_plus/mf_plus.h>

// Load the key at user-dictionary index `key_index` into `key`. Returns false if the index no
// longer exists (e.g. the file changed under us).
static bool nfc_scene_mf_plus_keys_delete_load_key(uint32_t key_index, MfPlusKey* key) {
    KeysDict* user_dict =
        keys_dict_alloc(NFC_APP_MF_PLUS_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfPlusKey));
    bool loaded = key_index < keys_dict_get_total_keys(user_dict);
    for(size_t i = 0; loaded && i < (key_index + 1); i++) {
        loaded = keys_dict_get_next_key(user_dict, key->data, sizeof(MfPlusKey));
    }
    keys_dict_free(user_dict);
    return loaded;
}

void nfc_scene_mf_plus_keys_delete_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_scene_mf_plus_keys_delete_on_enter(void* context) {
    NfcApp* instance = context;

    uint32_t key_index =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfPlusKeysDelete);

    widget_add_string_element(
        instance->widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Delete this key?");
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeLeft,
        "Cancel",
        nfc_scene_mf_plus_keys_delete_widget_callback,
        instance);
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeRight,
        "Delete",
        nfc_scene_mf_plus_keys_delete_widget_callback,
        instance);

    MfPlusKey stack_key;
    if(nfc_scene_mf_plus_keys_delete_load_key(key_index, &stack_key)) {
        FuriString* key_str = furi_string_alloc();
        for(size_t i = 0; i < sizeof(MfPlusKey); i++) {
            furi_string_cat_printf(key_str, "%02X", stack_key.data[i]);
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
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_plus_keys_delete_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            uint32_t key_index =
                scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfPlusKeysDelete);
            MfPlusKey stack_key;
            bool deleted = false;
            if(nfc_scene_mf_plus_keys_delete_load_key(key_index, &stack_key)) {
                KeysDict* user_dict = keys_dict_alloc(
                    NFC_APP_MF_PLUS_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfPlusKey));
                deleted = keys_dict_delete_key(user_dict, stack_key.data, sizeof(MfPlusKey));
                keys_dict_free(user_dict);
            }
            if(deleted) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneDeleteSuccess);
            } else {
                scene_manager_previous_scene(instance->scene_manager);
            }
        } else if(event.event == GuiButtonTypeLeft) {
            scene_manager_previous_scene(instance->scene_manager);
        }
        consumed = true;
    }

    return consumed;
}

void nfc_scene_mf_plus_keys_delete_on_exit(void* context) {
    NfcApp* instance = context;

    widget_reset(instance->widget);
}
