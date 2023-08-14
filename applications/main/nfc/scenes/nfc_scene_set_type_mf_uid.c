#include "../nfc_i.h"
#include "lib/nfc/helpers/nfc_generators.h"

enum SubmenuIndex {
    SubmenuIndexMFC1k4b,
    SubmenuIndexMFC4k4b,
    SubmenuIndexMFC1k7b,
    SubmenuIndexMFC4k7b,
    SubmenuIndexMFCMini,
};

static const NfcGenerator ganeator_gag = {
    .name = "Mifare Classic Custom UID",
    .generator_func = NULL,
};

void nfc_scene_set_type_mf_uid_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_set_type_mf_uid_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu,
        "Mifare Classic 1k 4byte UID",
        SubmenuIndexMFC1k4b,
        nfc_scene_set_type_mf_uid_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Mifare Classic 4k 4byte UID",
        SubmenuIndexMFC4k4b,
        nfc_scene_set_type_mf_uid_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Mifare Classic 1k 7byte UID",
        SubmenuIndexMFC1k7b,
        nfc_scene_set_type_mf_uid_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Mifare Classic 4k 7byte UID",
        SubmenuIndexMFC4k7b,
        nfc_scene_set_type_mf_uid_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Mifare Classic Mini",
        SubmenuIndexMFCMini,
        nfc_scene_set_type_mf_uid_submenu_callback,
        nfc);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_set_type_mf_uid_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;
    bool correct_index = false;
    MfClassicType mf_type = MfClassicType1k;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexMFC1k4b) {
            nfc->dev->dev_data.nfc_data.uid_len = 4;
            mf_type = MfClassicType1k;
            correct_index = true;
        } else if(event.event == SubmenuIndexMFC1k7b) {
            nfc->dev->dev_data.nfc_data.uid_len = 7;
            mf_type = MfClassicType1k;
            correct_index = true;
        } else if(event.event == SubmenuIndexMFC4k4b) {
            nfc->dev->dev_data.nfc_data.uid_len = 4;
            mf_type = MfClassicType4k;
            correct_index = true;
        } else if(event.event == SubmenuIndexMFC4k7b) {
            nfc->dev->dev_data.nfc_data.uid_len = 7;
            mf_type = MfClassicType4k;
            correct_index = true;
        } else if(event.event == SubmenuIndexMFCMini) {
            nfc->dev->dev_data.nfc_data.uid_len = 4;
            mf_type = MfClassicTypeMini;
            correct_index = true;
        }
        if(correct_index) {
            nfc->generator = &ganeator_gag;
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneSetTypeMfUid, mf_type);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetUid);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_set_type_mf_uid_on_exit(void* context) {
    Nfc* nfc = context;

    submenu_reset(nfc->submenu);
}
