#include "../nfc_app_i.h"

void nfc_scene_mf_ultralight_aes_keys_delete_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_scene_mf_ultralight_aes_keys_delete_on_enter(void* context) {
    NfcApp* instance = context;

    uint32_t key_index =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfUltralightAesKeysDelete);
    FuriString* key_str = furi_string_alloc();

    widget_add_string_element(
        instance->widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Delete this key?");
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeLeft,
        "Cancel",
        nfc_scene_mf_ultralight_aes_keys_delete_widget_callback,
        instance);
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeRight,
        "Delete",
        nfc_scene_mf_ultralight_aes_keys_delete_widget_callback,
        instance);

    KeysDict* user_dict = keys_dict_alloc(
        NFC_APP_MF_ULTRALIGHT_AES_DICT_USER_PATH,
        KeysDictModeOpenAlways,
        sizeof(MfUltralightAesKey));
    size_t dict_keys_num = keys_dict_get_total_keys(user_dict);
    furi_assert(key_index < dict_keys_num);
    MfUltralightAesKey stack_key;
    for(size_t i = 0; i < (key_index + 1); i++) {
        bool key_loaded =
            keys_dict_get_next_key(user_dict, stack_key.data, sizeof(MfUltralightAesKey));
        furi_assert(key_loaded);
    }
    furi_string_reset(key_str);
    for(size_t i = 0; i < sizeof(MfUltralightAesKey); i++) {
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

    keys_dict_free(user_dict);
    furi_string_free(key_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_ultralight_aes_keys_delete_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            uint32_t key_index = scene_manager_get_scene_state(
                instance->scene_manager, NfcSceneMfUltralightAesKeysDelete);
            KeysDict* user_dict = keys_dict_alloc(
                NFC_APP_MF_ULTRALIGHT_AES_DICT_USER_PATH,
                KeysDictModeOpenAlways,
                sizeof(MfUltralightAesKey));
            size_t dict_keys_num = keys_dict_get_total_keys(user_dict);
            furi_assert(key_index < dict_keys_num);
            MfUltralightAesKey stack_key;
            for(size_t i = 0; i < (key_index + 1); i++) {
                bool key_loaded =
                    keys_dict_get_next_key(user_dict, stack_key.data, sizeof(MfUltralightAesKey));
                furi_assert(key_loaded);
            }
            bool key_delete_success =
                keys_dict_delete_key(user_dict, stack_key.data, sizeof(MfUltralightAesKey));
            keys_dict_free(user_dict);
            if(key_delete_success) {
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

void nfc_scene_mf_ultralight_aes_keys_delete_on_exit(void* context) {
    NfcApp* instance = context;

    widget_reset(instance->widget);
}
