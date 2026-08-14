#include "../nfc_app_i.h"

static uint32_t nfc_scene_key_dict_count(const char* path, KeysDictMode mode, size_t key_size) {
    KeysDict* dict = keys_dict_alloc(path, mode, key_size);
    uint32_t total = keys_dict_get_total_keys(dict);
    keys_dict_free(dict);

    return total;
}

void nfc_scene_key_dict_widget_callback(GuiButtonType result, InputType type, void* context) {
    NfcApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_scene_key_dict_on_enter(void* context) {
    NfcApp* instance = context;
    const NfcKeyDict* dict = nfc_key_dict(instance->key_dict_type);

    const uint32_t system_keys_total =
        nfc_scene_key_dict_count(dict->system_path, KeysDictModeOpenExisting, dict->key_size);
    const uint32_t user_keys_total =
        nfc_scene_key_dict_count(dict->user_path, KeysDictModeOpenAlways, dict->key_size);

    FuriString* temp_str = furi_string_alloc();
    widget_add_string_element(
        instance->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, dict->title);
    furi_string_printf(temp_str, "System dict: %lu", system_keys_total);
    widget_add_string_element(
        instance->widget,
        0,
        20,
        AlignLeft,
        AlignTop,
        FontSecondary,
        furi_string_get_cstr(temp_str));
    furi_string_printf(temp_str, "User dict: %lu", user_keys_total);
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
        instance->widget, GuiButtonTypeCenter, "Add", nfc_scene_key_dict_widget_callback, instance);
    if(user_keys_total > 0) {
        widget_add_button_element(
            instance->widget,
            GuiButtonTypeRight,
            "List",
            nfc_scene_key_dict_widget_callback,
            instance);
    }
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_key_dict_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneKeyDictAdd);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneKeyDictList);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_key_dict_on_exit(void* context) {
    NfcApp* instance = context;

    widget_reset(instance->widget);
}
