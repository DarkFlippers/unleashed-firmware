#include "../nfc_i.h"

void nfc_scene_mf_classic_keys_widget_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_mf_classic_keys_on_enter(void* context) {
    Nfc* nfc = context;

    // Load flipper dict keys total
    uint32_t flipper_dict_keys_total = 0;
    MfClassicDict* dict = mf_classic_dict_alloc(MfClassicDictTypeFlipper);
    if(dict) {
        flipper_dict_keys_total = mf_classic_dict_get_total_keys(dict);
        mf_classic_dict_free(dict);
    }
    // Load user dict keys total
    uint32_t user_dict_keys_total = 0;
    dict = mf_classic_dict_alloc(MfClassicDictTypeUser);
    if(dict) {
        user_dict_keys_total = mf_classic_dict_get_total_keys(dict);
        mf_classic_dict_free(dict);
    }

    widget_add_string_element(
        nfc->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Mifare Classic Keys");
    char temp_str[32];
    snprintf(temp_str, sizeof(temp_str), "Flipper list: %ld", flipper_dict_keys_total);
    widget_add_string_element(nfc->widget, 0, 20, AlignLeft, AlignTop, FontSecondary, temp_str);
    snprintf(temp_str, sizeof(temp_str), "User list: %ld", user_dict_keys_total);
    widget_add_string_element(nfc->widget, 0, 32, AlignLeft, AlignTop, FontSecondary, temp_str);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeCenter, "Add", nfc_scene_mf_classic_keys_widget_callback, nfc);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeLeft, "Back", nfc_scene_mf_classic_keys_widget_callback, nfc);
    widget_add_icon_element(nfc->widget, 87, 13, &I_Keychain_39x36);
    if(user_dict_keys_total > 0) {
        widget_add_button_element(
            nfc->widget,
            GuiButtonTypeRight,
            "List",
            nfc_scene_mf_classic_keys_widget_callback,
            nfc);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_keys_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicKeysAdd);
            consumed = true;
        } else if(event.event == GuiButtonTypeLeft) {
            scene_manager_previous_scene(nfc->scene_manager);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicKeysList);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_mf_classic_keys_on_exit(void* context) {
    Nfc* nfc = context;

    widget_reset(nfc->widget);
}
