#include "../nfc_i.h"
#include "lib/nfc/helpers/nfc_generators.h"

enum SubmenuIndex {
    SubmenuIndexNFCA4,
    SubmenuIndexNFCA7,
    SubmenuIndexMFC1k4Uid,
    SubmenuIndexMFC4k4Uid,
    SubmenuIndexMFC1k7Uid,
    SubmenuIndexMFC4k7Uid,
    SubmenuIndexMFCMini,
    SubmenuIndexGeneratorsStart,
};

static const NfcGenerator ganeator_gag = {
    .name = "Mifare Classic Custom UID",
    .generator_func = NULL,
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
        "MFClassic1k4b Custom uid",
        SubmenuIndexMFC1k4Uid,
        nfc_scene_set_type_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "MFClassic4k4b Custom uid",
        SubmenuIndexMFC4k4Uid,
        nfc_scene_set_type_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "MFClassic1k7b Custom uid",
        SubmenuIndexMFC1k7Uid,
        nfc_scene_set_type_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "MFClassic4k7b Custom uid ",
        SubmenuIndexMFC4k7Uid,
        nfc_scene_set_type_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "MFClassic Mini Custom uid ",
        SubmenuIndexMFCMini,
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
        } else if(event.event == SubmenuIndexMFC1k4Uid) {
            nfc->dev->dev_data.nfc_data.uid_len = 4;
            nfc->dev->format = NfcDeviceSaveFormatMifareClassic;
            nfc->generator = &ganeator_gag;
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneSetUid, NfcSceneSetUidStateMFClassic1k);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetUid);
            consumed = true;
        } else if(event.event == SubmenuIndexMFC1k7Uid) {
            nfc->dev->dev_data.nfc_data.uid_len = 7;
            nfc->dev->format = NfcDeviceSaveFormatMifareClassic;
            nfc->generator = &ganeator_gag;
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneSetUid, NfcSceneSetUidStateMFClassic1k);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetUid);
            consumed = true;
        } else if(event.event == SubmenuIndexMFC4k4Uid) {
            nfc->dev->dev_data.nfc_data.uid_len = 4;
            nfc->dev->format = NfcDeviceSaveFormatMifareClassic;
            nfc->generator = &ganeator_gag;
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneSetUid, NfcSceneSetUidStateMFClassic4k);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetUid);
            consumed = true;
        } else if(event.event == SubmenuIndexMFC4k7Uid) {
            nfc->dev->dev_data.nfc_data.uid_len = 7;
            nfc->dev->format = NfcDeviceSaveFormatMifareClassic;
            nfc->generator = &ganeator_gag;
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneSetUid, NfcSceneSetUidStateMFClassic4k);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetUid);
            consumed = true;
        } else if(event.event == SubmenuIndexMFCMini) {
            nfc->dev->dev_data.nfc_data.uid_len = 4;
            nfc->dev->format = NfcDeviceSaveFormatMifareClassic;
            nfc->generator = &ganeator_gag;
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneSetUid, NfcSceneSetUidStateMFClassicMini);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetUid);
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
