#include "../nfc_app_i.h"

void nfc_scene_mf_classic_keys_delete_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_scene_mf_classic_keys_delete_on_enter(void* context) {
    NfcApp* instance = context;

    uint32_t key_index =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicKeysDelete);
    FuriString* key_str = furi_string_alloc();

    widget_add_string_element(
        instance->widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Delete this key?");
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeLeft,
        "Cancel",
        nfc_scene_mf_classic_keys_delete_widget_callback,
        instance);
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeRight,
        "Delete",
        nfc_scene_mf_classic_keys_delete_widget_callback,
        instance);

    mf_user_dict_get_key_str(instance->mf_user_dict, key_index, key_str);
    widget_add_string_element(
        instance->widget,
        64,
        32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        furi_string_get_cstr(key_str));

    furi_string_free(key_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_keys_delete_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            uint32_t key_index = scene_manager_get_scene_state(
                instance->scene_manager, NfcSceneMfClassicKeysDelete);
            if(mf_user_dict_delete_key(instance->mf_user_dict, key_index)) {
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

void nfc_scene_mf_classic_keys_delete_on_exit(void* context) {
    NfcApp* instance = context;

    mf_user_dict_free(instance->mf_user_dict);
    widget_reset(instance->widget);
}
