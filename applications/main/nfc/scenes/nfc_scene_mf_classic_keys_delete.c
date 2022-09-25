#include "../nfc_i.h"

void nfc_scene_mf_classic_keys_delete_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_mf_classic_keys_delete_on_enter(void* context) {
    Nfc* nfc = context;
    MfClassicDict* dict = mf_classic_dict_alloc(MfClassicDictTypeUser);
    uint32_t key_index =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfClassicKeysDelete);
    // Setup Custom Widget view
    string_t key_str;
    string_init(key_str);

    widget_add_string_element(
        nfc->widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Delete this key?");
    widget_add_button_element(
        nfc->widget,
        GuiButtonTypeLeft,
        "Cancel",
        nfc_scene_mf_classic_keys_delete_widget_callback,
        nfc);
    widget_add_button_element(
        nfc->widget,
        GuiButtonTypeRight,
        "Delete",
        nfc_scene_mf_classic_keys_delete_widget_callback,
        nfc);

    mf_classic_dict_get_key_at_index_str(dict, key_str, key_index);
    widget_add_string_element(
        nfc->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, string_get_cstr(key_str));

    string_clear(key_str);
    mf_classic_dict_free(dict);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_keys_delete_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;
    uint32_t key_index =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfClassicKeysDelete);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                nfc->scene_manager, NfcSceneMfClassicKeys);
        } else if(event.event == GuiButtonTypeRight) {
            MfClassicDict* dict = mf_classic_dict_alloc(MfClassicDictTypeUser);
            if(mf_classic_dict_delete_index(dict, key_index)) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneDeleteSuccess);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneMfClassicKeys);
            }
            mf_classic_dict_free(dict);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_mf_classic_keys_delete_on_exit(void* context) {
    Nfc* nfc = context;

    widget_reset(nfc->widget);
}
