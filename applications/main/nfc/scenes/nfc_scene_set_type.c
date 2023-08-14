#include "../nfc_i.h"
#include "lib/nfc/helpers/nfc_generators.h"

enum SubmenuIndex {
    SubmenuIndexNFCA4,
    SubmenuIndexNFCA7,
    SubmenuIndexMFClassicCustomUID,
    SubmenuIndexGeneratorsStart,
};

void nfc_scene_set_type_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_set_type_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;
    // Clear device name
    nfc_device_set_name(nfc->dev, "");
    furi_string_set(nfc->dev->load_path, "");
    submenu_add_item(
        submenu, "NFC-A 7-bytes UID", SubmenuIndexNFCA7, nfc_scene_set_type_submenu_callback, nfc);
    submenu_add_item(
        submenu, "NFC-A 4-bytes UID", SubmenuIndexNFCA4, nfc_scene_set_type_submenu_callback, nfc);
    submenu_add_item(
        submenu,
        "Mifare Classic Custom UID",
        SubmenuIndexMFClassicCustomUID,
        nfc_scene_set_type_submenu_callback,
        nfc);

    // Generators
    int i = SubmenuIndexGeneratorsStart;
    for(const NfcGenerator* const* generator = nfc_generators; *generator != NULL;
        ++generator, ++i) {
        submenu_add_item(submenu, (*generator)->name, i, nfc_scene_set_type_submenu_callback, nfc);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_set_type_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexNFCA7) {
            nfc->dev->dev_data.nfc_data.uid_len = 7;
            nfc->dev->format = NfcDeviceSaveFormatUid;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetSak);
            consumed = true;
        } else if(event.event == SubmenuIndexNFCA4) {
            nfc->dev->dev_data.nfc_data.uid_len = 4;
            nfc->dev->format = NfcDeviceSaveFormatUid;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetSak);
            consumed = true;
        } else if(event.event == SubmenuIndexMFClassicCustomUID) {
            nfc_device_clear(nfc->dev);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetTypeMfUid);
            consumed = true;
        } else {
            nfc_device_clear(nfc->dev);
            nfc->generator = nfc_generators[event.event - SubmenuIndexGeneratorsStart];
            nfc->generator->generator_func(&nfc->dev->dev_data);

            scene_manager_next_scene(nfc->scene_manager, NfcSceneGenerateInfo);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_set_type_on_exit(void* context) {
    Nfc* nfc = context;

    submenu_reset(nfc->submenu);
}
