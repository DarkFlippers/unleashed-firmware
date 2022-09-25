#include "../nfc_i.h"

#define NFC_SCENE_MF_CLASSIC_KEYS_LIST_MAX (100)

void nfc_scene_mf_classic_keys_list_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);

    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_mf_classic_keys_list_popup_callback(void* context) {
    furi_assert(context);

    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
}

void nfc_scene_mf_classic_keys_list_prepare(Nfc* nfc, MfClassicDict* dict) {
    Submenu* submenu = nfc->submenu;
    uint32_t index = 0;
    string_t temp_key;
    string_init(temp_key);

    submenu_set_header(submenu, "Select key to delete:");
    while(mf_classic_dict_get_next_key_str(dict, temp_key)) {
        char* current_key = (char*)malloc(sizeof(char) * 13);
        strncpy(current_key, string_get_cstr(temp_key), 12);
        MfClassicUserKeys_push_back(nfc->mfc_key_strs, current_key);
        FURI_LOG_D("ListKeys", "Key %d: %s", index, current_key);
        submenu_add_item(
            submenu, current_key, index++, nfc_scene_mf_classic_keys_list_submenu_callback, nfc);
    }
    string_clear(temp_key);
}

void nfc_scene_mf_classic_keys_list_on_enter(void* context) {
    Nfc* nfc = context;
    MfClassicDict* dict = mf_classic_dict_alloc(MfClassicDictTypeUser);
    MfClassicUserKeys_init(nfc->mfc_key_strs);
    if(dict) {
        uint32_t total_user_keys = mf_classic_dict_get_total_keys(dict);
        if(total_user_keys < NFC_SCENE_MF_CLASSIC_KEYS_LIST_MAX) {
            nfc_scene_mf_classic_keys_list_prepare(nfc, dict);
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
        } else {
            popup_set_header(nfc->popup, "Too many keys!", 64, 0, AlignCenter, AlignTop);
            popup_set_text(
                nfc->popup,
                "Edit user dictionary\nwith file browser",
                64,
                12,
                AlignCenter,
                AlignTop);
            popup_set_callback(nfc->popup, nfc_scene_mf_classic_keys_list_popup_callback);
            popup_set_context(nfc->popup, nfc);
            popup_set_timeout(nfc->popup, 3000);
            popup_enable_timeout(nfc->popup);
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
        }
        mf_classic_dict_free(dict);
    } else {
        popup_set_header(
            nfc->popup, "Failed to load dictionary", 64, 32, AlignCenter, AlignCenter);
        popup_set_callback(nfc->popup, nfc_scene_mf_classic_keys_list_popup_callback);
        popup_set_context(nfc->popup, nfc);
        popup_set_timeout(nfc->popup, 3000);
        popup_enable_timeout(nfc->popup);
        view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
    }
}

bool nfc_scene_mf_classic_keys_list_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfClassicKeysDelete, event.event);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicKeysDelete);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_mf_classic_keys_list_on_exit(void* context) {
    Nfc* nfc = context;

    MfClassicUserKeys_it_t it;
    for(MfClassicUserKeys_it(it, nfc->mfc_key_strs); !MfClassicUserKeys_end_p(it);
        MfClassicUserKeys_next(it)) {
        free(*MfClassicUserKeys_ref(it));
    }
    MfClassicUserKeys_clear(nfc->mfc_key_strs);
    submenu_reset(nfc->submenu);
    popup_reset(nfc->popup);
}
