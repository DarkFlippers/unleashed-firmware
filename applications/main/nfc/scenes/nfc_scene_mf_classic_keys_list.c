#include "../nfc_app_i.h"

#define NFC_SCENE_MF_CLASSIC_KEYS_LIST_MAX (100)

void nfc_scene_mf_classic_keys_list_submenu_callback(void* context, uint32_t index) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, index);
}

void nfc_scene_mf_classic_keys_list_on_enter(void* context) {
    NfcApp* instance = context;

    instance->mf_user_dict = mf_user_dict_alloc(NFC_SCENE_MF_CLASSIC_KEYS_LIST_MAX);

    submenu_set_header(instance->submenu, "Select key to delete:");
    FuriString* temp_str = furi_string_alloc();
    for(size_t i = 0; i < mf_user_dict_get_keys_cnt(instance->mf_user_dict); i++) {
        mf_user_dict_get_key_str(instance->mf_user_dict, i, temp_str);
        submenu_add_item(
            instance->submenu,
            furi_string_get_cstr(temp_str),
            i,
            nfc_scene_mf_classic_keys_list_submenu_callback,
            instance);
    }
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mf_classic_keys_list_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;

    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(
            instance->scene_manager, NfcSceneMfClassicKeysDelete, event.event);
        scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicKeysDelete);
    } else if(event.type == SceneManagerEventTypeBack) {
        mf_user_dict_free(instance->mf_user_dict);
    }

    return consumed;
}

void nfc_scene_mf_classic_keys_list_on_exit(void* context) {
    NfcApp* instance = context;

    submenu_reset(instance->submenu);
}
