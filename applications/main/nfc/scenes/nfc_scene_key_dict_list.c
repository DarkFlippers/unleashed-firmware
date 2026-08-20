#include "../nfc_app_i.h"
#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"

#define TAG "NfcKeyDict"

#define NFC_SCENE_KEY_DICT_LIST_MAX (100)

void nfc_scene_key_dict_list_on_enter(void* context) {
    NfcApp* instance = context;
    const NfcKeyDict* dict = nfc_key_dict(instance->key_dict_type);

    KeysDict* user_dict = keys_dict_alloc(dict->user_path, KeysDictModeOpenAlways, dict->key_size);

    submenu_set_header(instance->submenu, "Select key to delete:");
    FuriString* temp_str = furi_string_alloc();

    const size_t keys_num =
        MIN((size_t)NFC_SCENE_KEY_DICT_LIST_MAX, keys_dict_get_total_keys(user_dict));
    uint8_t key[NFC_BYTE_INPUT_STORE_SIZE];

    for(size_t i = 0; i < keys_num; i++) {
        if(!keys_dict_get_next_key(user_dict, key, dict->key_size)) {
            // The count said there were more. Short list rather than no list, but say why.
            FURI_LOG_W(TAG, "Dictionary %s ended early at key %u", dict->user_path, (unsigned)i);
            break;
        }
        furi_string_reset(temp_str);
        for(size_t j = 0; j < dict->key_size; j++) {
            furi_string_cat_printf(temp_str, "%02X", key[j]);
        }
        submenu_add_item(
            instance->submenu,
            furi_string_get_cstr(temp_str),
            i,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }

    furi_string_free(temp_str);
    keys_dict_free(user_dict);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_key_dict_list_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;

    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(instance->scene_manager, NfcSceneKeyDictDelete, event.event);
        scene_manager_next_scene(instance->scene_manager, NfcSceneKeyDictDelete);
        consumed = true;
    }

    return consumed;
}

void nfc_scene_key_dict_list_on_exit(void* context) {
    NfcApp* instance = context;

    submenu_reset(instance->submenu);
}
