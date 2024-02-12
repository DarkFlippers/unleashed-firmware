#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"

enum SubmenuIndex {
    SubmenuIndexGeneratorsStart,
    SubmenuIndexNFCA4 = NfcDataGeneratorTypeNum,
    SubmenuIndexNFCA7,
};

static void nfc_scene_set_type_init_edit_data(Iso14443_3aData* data, size_t uid_len) {
    // Easiest way to create a zero'd buffer of given length
    uint8_t* uid = calloc(1, uid_len);
    iso14443_3a_set_uid(data, uid, uid_len);
    free(uid);
}

void nfc_scene_set_type_on_enter(void* context) {
    NfcApp* instance = context;

    Submenu* submenu = instance->submenu;
    submenu_add_item(
        submenu,
        "NFC-A 7-bytes UID",
        SubmenuIndexNFCA7,
        nfc_protocol_support_common_submenu_callback,
        instance);
    submenu_add_item(
        submenu,
        "NFC-A 4-bytes UID",
        SubmenuIndexNFCA4,
        nfc_protocol_support_common_submenu_callback,
        instance);

    FuriString* str = furi_string_alloc();
    for(size_t i = 0; i < NfcDataGeneratorTypeNum; i++) {
        furi_string_cat_str(str, nfc_data_generator_get_name(i));
        furi_string_replace_str(str, "Mifare", "MIFARE");

        submenu_add_item(
            submenu,
            furi_string_get_cstr(str),
            i,
            nfc_protocol_support_common_submenu_callback,
            instance);
        furi_string_reset(str);
    }
    furi_string_free(str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_set_type_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexNFCA7) {
            nfc_scene_set_type_init_edit_data(instance->iso14443_3a_edit_data, 7);
            scene_manager_next_scene(instance->scene_manager, NfcSceneSetSak);
            consumed = true;
        } else if(event.event == SubmenuIndexNFCA4) {
            nfc_scene_set_type_init_edit_data(instance->iso14443_3a_edit_data, 4);
            scene_manager_next_scene(instance->scene_manager, NfcSceneSetSak);
            consumed = true;
        } else {
            nfc_data_generator_fill_data(event.event, instance->nfc_device);
            scene_manager_set_scene_state(
                instance->scene_manager, NfcSceneGenerateInfo, event.event);
            scene_manager_next_scene(instance->scene_manager, NfcSceneGenerateInfo);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_set_type_on_exit(void* context) {
    NfcApp* instance = context;

    submenu_reset(instance->submenu);
}
