#include "../nfc_app_i.h"

#define NFC_SCENE_MF_CLASSIC_KEYS_MAX (100)

void nfc_scene_mf_classic_keys_widget_callback(GuiButtonType result, InputType type, void* context) {
    NfcApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_scene_mf_classic_keys_on_enter(void* context) {
    NfcApp* instance = context;

    // Load flipper dict keys total
    uint32_t flipper_dict_keys_total = 0;
    KeysDict* dict = keys_dict_alloc(
        NFC_APP_MF_CLASSIC_DICT_SYSTEM_PATH, KeysDictModeOpenExisting, sizeof(MfClassicKey));
    flipper_dict_keys_total = keys_dict_get_total_keys(dict);
    keys_dict_free(dict);

    // Load user dict keys total
    uint32_t user_dict_keys_total = 0;
    dict = keys_dict_alloc(
        NFC_APP_MF_CLASSIC_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfClassicKey));
    user_dict_keys_total = keys_dict_get_total_keys(dict);
    keys_dict_free(dict);

    FuriString* temp_str = furi_string_alloc();
    widget_add_string_element(
        instance->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "MIFARE Classic Keys");
    furi_string_printf(temp_str, "System dict: %lu", flipper_dict_keys_total);
    widget_add_string_element(
        instance->widget,
        0,
        20,
        AlignLeft,
        AlignTop,
        FontSecondary,
        furi_string_get_cstr(temp_str));
    furi_string_printf(temp_str, "User dict: %lu", user_dict_keys_total);
    widget_add_string_element(
        instance->widget,
        0,
        32,
        AlignLeft,
        AlignTop,
        FontSecondary,
        furi_string_get_cstr(temp_str));
    widget_add_icon_element(instance->widget, 87, 13, &I_Keychain_39x36);
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeCenter,
        "Add",
        nfc_scene_mf_classic_keys_widget_callback,
        instance);
    if(user_dict_keys_total > 0) {
        widget_add_button_element(
            instance->widget,
            GuiButtonTypeRight,
            "List",
            nfc_scene_mf_classic_keys_widget_callback,
            instance);
    }
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_keys_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicKeysAdd);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicKeysList);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_mf_classic_keys_on_exit(void* context) {
    NfcApp* instance = context;

    widget_reset(instance->widget);
}
