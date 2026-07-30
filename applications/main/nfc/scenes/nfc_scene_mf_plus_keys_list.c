#include "../nfc_app_i.h"

#include <nfc/protocols/mf_plus/mf_plus.h>

#define NFC_SCENE_MF_PLUS_KEYS_LIST_MAX (100)

void nfc_scene_mf_plus_keys_list_submenu_callback(void* context, uint32_t index) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, index);
}

void nfc_scene_mf_plus_keys_list_on_enter(void* context) {
    NfcApp* instance = context;

    KeysDict* user_dict =
        keys_dict_alloc(NFC_APP_MF_PLUS_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfPlusKey));

    submenu_set_header(instance->submenu, "Select key to delete:");
    FuriString* temp_str = furi_string_alloc();

    size_t dict_keys_num = keys_dict_get_total_keys(user_dict);
    size_t keys_num = MIN((size_t)NFC_SCENE_MF_PLUS_KEYS_LIST_MAX, dict_keys_num);
    MfPlusKey stack_key;

    for(size_t i = 0; i < keys_num; i++) {
        bool key_loaded = keys_dict_get_next_key(user_dict, stack_key.data, sizeof(MfPlusKey));
        furi_assert(key_loaded);
        furi_string_reset(temp_str);
        for(size_t j = 0; j < sizeof(MfPlusKey); j++) {
            furi_string_cat_printf(temp_str, "%02X", stack_key.data[j]);
        }
        submenu_add_item(
            instance->submenu,
            furi_string_get_cstr(temp_str),
            i,
            nfc_scene_mf_plus_keys_list_submenu_callback,
            instance);
    }
    keys_dict_free(user_dict);
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mf_plus_keys_list_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;

    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(
            instance->scene_manager, NfcSceneMfPlusKeysDelete, event.event);
        scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusKeysDelete);
    }

    return consumed;
}

void nfc_scene_mf_plus_keys_list_on_exit(void* context) {
    NfcApp* instance = context;

    submenu_reset(instance->submenu);
}
